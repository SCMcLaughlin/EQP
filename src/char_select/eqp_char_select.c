
#include "eqp_char_select.h"

void char_select_init(R(CharSelect*) charSelect, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath)
{
    charSelect->serverStatus        = EQP_CHAR_SELECT_SERVER_LOCKED;
    charSelect->serverPlayerCount   = 0;
    
    timer_pool_init(B(charSelect), &charSelect->timerPool);
    
    ipc_set_open(B(charSelect), &charSelect->ipcSet, EQP_SOURCE_ID_CHAR_SELECT, ipcPath, masterIpcPath, logWriterIpcPath);
    
    core_init(C(charSelect), EQP_SOURCE_ID_CHAR_SELECT, ipc_set_log_writer_ipc(&charSelect->ipcSet));
    
    charSelect->L = lua_sys_open(B(charSelect));
    lua_sys_run_file(B(charSelect), charSelect->L, EQP_CHAR_SELECT_SCRIPT_CHAR_CREATE, 0);
    
    charSelect->socket = udp_socket_create(B(charSelect));
    udp_socket_open(charSelect->socket, EQP_CHAR_SELECT_PORT);
    
    charSelect->loginServerConnections = array_create_type(B(charSelect), TcpClient);
    
    timer_init(&charSelect->timerUnclaimedAuths, &charSelect->timerPool, EQP_CHAR_SELECT_UNCLAIMED_AUTHS_TIMEOUT,
        char_select_unclaimed_auths_timer_callback, charSelect, true);
    
    charSelect->unclaimedAuths              = array_create_type(B(charSelect), CharSelectAuth);
    charSelect->unauthedClients             = array_create_type(B(charSelect), CharSelectClient*);
    charSelect->clientsAttemptingToZoneIn   = array_create_type(B(charSelect), CharSelectClientAttemptingZoneIn);
}

void char_select_deinit(R(CharSelect*) charSelect)
{
    core_deinit(C(charSelect));
    ipc_set_deinit(&charSelect->ipcSet);
    
    if (charSelect->L)
    {
        lua_close(charSelect->L);
        charSelect->L = NULL;
    }
    
    if (charSelect->socket)
    {
        udp_socket_destroy(charSelect->socket);
        charSelect->socket = NULL;
    }
    
    if (charSelect->loginServerConnections)
    {
        R(TcpClient*) array = array_data_type(charSelect->loginServerConnections, TcpClient);
        uint32_t n          = array_count(charSelect->loginServerConnections);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            tcp_client_deinit(&array[i]);
        }
        
        array_destroy(charSelect->loginServerConnections);
        charSelect->loginServerConnections = NULL;
    }
    
    if (charSelect->unclaimedAuths)
    {
        array_destroy(charSelect->unclaimedAuths);
        charSelect->unclaimedAuths = NULL;
    }
    
    if (charSelect->unauthedClients)
    {
        array_destroy(charSelect->unauthedClients);
        charSelect->unauthedClients = NULL;
    }
    
    if (charSelect->clientsAttemptingToZoneIn)
    {
        array_destroy(charSelect->clientsAttemptingToZoneIn);
        charSelect->clientsAttemptingToZoneIn = NULL;
    }
}

static void char_select_handle_op_client_zoning(R(CharSelect*) charSelect, R(byte*) data, uint32_t length)
{
    R(Server_ZoneAddress*) zoneAddr;
    R(CharSelectClientAttemptingZoneIn*) array;
    uint32_t accountId;
    uint32_t n;
    uint32_t i;
    
    if (length < sizeof(Server_ZoneAddress))
        return;
    
    zoneAddr = (Server_ZoneAddress*)data;
    
    if (length < (offsetof(Server_ZoneAddress, messageOfTheDay) + zoneAddr->motdLength))
        return;
    
    accountId   = zoneAddr->accountId;
    array       = array_data_type(charSelect->clientsAttemptingToZoneIn, CharSelectClientAttemptingZoneIn);
    n           = array_count(charSelect->clientsAttemptingToZoneIn);
    
    for (i = 0; i < n; i++)
    {
        if (array[i].accountId == accountId)
        {
            char_select_client_on_zone_in_success(array[i].client, charSelect, zoneAddr);
            array_swap_and_pop(charSelect->clientsAttemptingToZoneIn, i);
            break;
        }
    }
}

static void char_select_handle_op_client_zoning_rejected(R(CharSelect*) charSelect, R(byte*) data, uint32_t length)
{
    R(Server_ClientZoningReject*) reject;
    R(CharSelectClientAttemptingZoneIn*) array;
    uint32_t accountId;
    uint32_t n;
    uint32_t i;
    
    if (length < sizeof(Server_ClientZoningReject))
        return;
    
    reject      = (Server_ClientZoningReject*)data;
    accountId   = reject->accountId;
    array       = array_data_type(charSelect->clientsAttemptingToZoneIn, CharSelectClientAttemptingZoneIn);
    n           = array_count(charSelect->clientsAttemptingToZoneIn);
    
    for (i = 0; i < n; i++)
    {
        if (array[i].accountId == accountId)
        {
            char_select_client_on_zone_in_failure(array[i].client, charSelect, reject->zoneShortName);
            array_swap_and_pop(charSelect->clientsAttemptingToZoneIn, i);
            break;
        }
    }
}

void ipc_set_handle_packet(R(Basic*) basic, R(IpcPacket*) packet)
{
    R(CharSelect*) charSelect   = (CharSelect*)basic;
    ServerOp opcode             = ipc_packet_opcode(packet);
    uint32_t length             = ipc_packet_length(packet);
    R(byte*) data               = ipc_packet_data(packet);
    
    switch (opcode)
    {
    case ServerOp_ZoneAddress:
        char_select_handle_op_client_zoning(charSelect, data, length);
        break;
    
    case ServerOp_ClientZoningReject:
        char_select_handle_op_client_zoning_rejected(charSelect, data, length);
        break;
    
    default:
        break;
    }
}

void char_select_main_loop(R(CharSelect*) charSelect)
{
    R(UdpSocket*) socket    = charSelect->socket;
    R(IpcSet*) ipcSet       = &charSelect->ipcSet;
    
    for (;;)
    {
        udp_socket_recv(socket);
        
        db_thread_execute_query_callbacks(core_db_thread(C(charSelect)));
        timer_pool_execute_callbacks(&charSelect->timerPool);
        
        udp_socket_send(socket);
        udp_socket_check_timeouts(socket);
        
        char_select_tcp_recv(charSelect);
        
        if (ipc_set_receive(B(charSelect), ipcSet))
        {
            log_format(B(charSelect), LogInfo, "Shutting down cleanly");
            break;
        }
        
        ipc_set_keep_alive(B(charSelect), ipcSet, EQP_SOURCE_ID_CHAR_SELECT);
        
        clock_sleep_milliseconds(50);
    }
}

void char_select_start_login_server_connections(R(CharSelect*) charSelect)
{
    R(lua_State*) L = charSelect->L;
    uint32_t n;
    uint32_t i;
    
    lua_sys_run_file(B(charSelect), L, EQP_CHAR_SELECT_LOGIN_CONFIG, 1);
    
    n = lua_objlen(L, -1);
    
    for (i = 1; i <= n; i++)
    {
        R(TcpClient*) client;
        R(LoginServerConfig*) server = eqp_alloc_type(B(charSelect), LoginServerConfig);
        
        lua_pushinteger(L, i);
        lua_gettable(L, -2);
        
        server->index       = i - 1;
        server->charSelect  = charSelect;
        server->longName    = lua_sys_field_to_string(B(charSelect), L, -1, "longname");
        server->shortName   = lua_sys_field_to_string(B(charSelect), L, -1, "shortname");
        server->host        = lua_sys_field_to_string(B(charSelect), L, -1, "host");
        server->port        = lua_sys_field_to_string(B(charSelect), L, -1, "port");
        server->username    = lua_sys_field_to_string(B(charSelect), L, -1, "username");
        server->password    = lua_sys_field_to_string(B(charSelect), L, -1, "password");
        server->remoteIp    = lua_sys_field_to_string(B(charSelect), L, -1, "remoteaddress");
        server->localIp     = lua_sys_field_to_string(B(charSelect), L, -1, "localaddress");
        
        // Push new TcpClient object
        client = array_push_back_type(B(charSelect), &charSelect->loginServerConnections, TcpClient);
        
        tcp_client_init(charSelect, client, server);
        tcp_client_start_connect_cycle(client, true);
        
        lua_pop(L, 1);
    }
    
    lua_clear(L);
}

void char_select_tcp_recv(R(CharSelect*) charSelect)
{
    R(TcpClient*) array = array_data_type(charSelect->loginServerConnections, TcpClient);
    uint32_t n          = array_count(charSelect->loginServerConnections);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        R(TcpClient*) cli   = &array[i];
        int fd              = tcp_client_fd(cli);
        int buffered;
        int readLength;
        R(byte*) recvBuf;
        int len;
        
        if (fd == INVALID_SOCKET)
            continue;
        
        buffered    = tcp_client_buffered(cli);
        readLength  = tcp_client_read_length(cli);
        recvBuf     = tcp_client_recv_buffer(cli);
        
    redo:
        len = recv(fd, recvBuf + buffered, readLength - buffered, 0);
        
        if (len == -1)
        {
            int err = errno;
            if (err != EAGAIN && err != EWOULDBLOCK)
                log_format(B(charSelect), LogNetwork, "[char_select_tcp_recv] recv() syscall failed, errno: %i", err);
            
            continue;
        }
        
        if (len == 0)
        {
            // Remote end has closed the connection
            tcp_client_restart_connection(cli);
            continue;
        }
        
        buffered += len;
        
        if (buffered == readLength)
        {
            if (buffered == sizeof(TcpPacketHeader))
            {
                readLength = *(uint16_t*)(&recvBuf[2]); // Aligned read
                if (readLength == sizeof(TcpPacketHeader))
                    goto packet;
                tcp_client_set_read_length(cli, readLength);
            }
            else
            {
            packet:
                // We have a complete packet to handle
                tcp_client_handle_packet(cli);
                
                buffered    = 0;
                readLength  = sizeof(TcpPacketHeader);
                tcp_client_set_buffered(cli, 0);
                tcp_client_set_read_length(cli, sizeof(TcpPacketHeader));
            }
            
            goto redo;
        }
        
        tcp_client_set_buffered(cli, buffered);
    }
}

TcpClient* char_select_get_tcp_client(R(CharSelect*) charSelect, uint32_t index)
{
    return array_get_type(charSelect->loginServerConnections, index, TcpClient);
}

void char_select_unclaimed_auths_timer_callback(R(Timer*) timer)
{
    CharSelect* charSelect  = timer_userdata_type(timer, CharSelect);
    CharSelectAuth* array   = array_data_type(charSelect->unclaimedAuths, CharSelectAuth);
    uint32_t n              = array_count(charSelect->unclaimedAuths);
    uint32_t i              = 0;
    uint64_t time           = clock_milliseconds();
    
    while (i < n)
    {
        CharSelectAuth* auth = &array[i];
        
        if ((time - auth->timestamp) >= EQP_CHAR_SELECT_UNCLAIMED_AUTHS_TIMEOUT)
        {
            array_swap_and_pop(charSelect->unclaimedAuths, i);
            n--;
            continue;
        }
        
        i++;
    }
}

void char_select_handle_client_auth(R(CharSelect*) charSelect, R(CharSelectAuth*) auth)
{
    uint32_t accountId          = auth->accountId;
    R(const char*) sessionKey   = auth->sessionKey;
    R(CharSelectClient**) array = array_data_type(charSelect->unauthedClients, CharSelectClient*);
    uint32_t n                  = array_count(charSelect->unauthedClients);
    uint32_t i;
    
    // Check if there's an unauthed client waiting for this
    for (i = 0; i < n; i++)
    {
        R(CharSelectClient*) client = array[i];
        
        if (char_select_client_account_id(client) == accountId && strcmp(char_select_client_session_key(client), sessionKey) == 0)
        {
            char_select_client_set_auth(client, auth);
            array_swap_and_pop(charSelect->unauthedClients, i);
            return;
        }
    }
    
    // None found, append to our queue
    auth->timestamp = clock_milliseconds();
    array_push_back(B(charSelect), &charSelect->unclaimedAuths, auth);
}

void char_select_handle_unauthed_client(R(CharSelect*) charSelect, R(CharSelectClient*) client)
{
    uint32_t accountId          = char_select_client_account_id(client);
    R(const char*) sessionKey   = char_select_client_session_key(client);
    R(CharSelectAuth*) array    = array_data_type(charSelect->unclaimedAuths, CharSelectAuth);
    uint32_t n                  = array_count(charSelect->unclaimedAuths);
    uint32_t i;
    
    // Check if we've already received an auth for this client
    for (i = 0; i < n; i++)
    {
        R(CharSelectAuth*) auth = &array[i];
        
        if (auth->accountId == accountId && strcmp(auth->sessionKey, sessionKey) == 0)
        {
            char_select_client_set_auth(client, auth);
            array_swap_and_pop(charSelect->unclaimedAuths, i);
            return;
        }
    }
    
    // None found, append to our queue
    array_push_back(B(charSelect), &charSelect->unauthedClients, (void*)&client);
}

void char_select_remove_client_from_unauthed_list(R(CharSelect*) charSelect, R(CharSelectClient*) client)
{
    R(CharSelectClient**) array = array_data_type(charSelect->unauthedClients, CharSelectClient*);
    uint32_t n                  = array_count(charSelect->unauthedClients);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        if (array[i] == client)
        {
            array_swap_and_pop(charSelect->unauthedClients, i);
            return;
        }
    }
}

static void char_select_client_zone_in_timeout_callback(R(Timer*) timer)
{
    R(CharSelectClient*) client                 = timer_userdata_type(timer, CharSelectClient);
    R(CharSelect*) charSelect                   = (CharSelect*)protocol_handler_basic(char_select_client_handler(client));
    R(CharSelectClientAttemptingZoneIn*) array  = array_data_type(charSelect->clientsAttemptingToZoneIn, CharSelectClientAttemptingZoneIn);
    uint32_t n                                  = array_count(charSelect->clientsAttemptingToZoneIn);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        if (array[i].client == client)
        {
            array_swap_and_pop(charSelect->clientsAttemptingToZoneIn, i);
            char_select_client_on_zone_in_failure(client, charSelect, "qeynos"); //fixme: get the real shortname? may not be worth the bother
            break;
        }
    }
    
    char_select_client_drop(client);
    timer_destroy(timer);
}

void char_select_send_client_zone_in_request(R(CharSelect*) charSelect, R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(const char*) charName)
{
    CharSelectClientAttemptingZoneIn record;
    Server_ClientZoning zoning;
    
    zoning.accountId    = char_select_client_account_id(client);
    zoning.ipAddress    = protocol_handler_ip_address(handler)->sin_addr.s_addr;
    zoning.characterId  = 0;
    zoning.isLocal      = char_select_client_is_local(client);
    snprintf(zoning.accountName, sizeof(zoning.accountName), "%s", char_select_client_account_name(client));
    snprintf(zoning.characterName, sizeof(zoning.characterName), "%s", charName);
    
    ipc_set_send(B(charSelect), &charSelect->ipcSet, ServerOp_ClientZoning, EQP_SOURCE_ID_CHAR_SELECT, sizeof(zoning), &zoning);
    
    record.accountId    = zoning.accountId;
    record.client       = client;
    
    array_push_back(B(charSelect), &charSelect->clientsAttemptingToZoneIn, &record);
    
    char_select_client_grab(client);
    
    // Single-run timer
    eqp_timer_create(B(charSelect), char_select_timer_pool(charSelect), EQP_CHAR_SELECT_ZONE_ATTEMPT_TIMEOUT,
        char_select_client_zone_in_timeout_callback, client, true);
}

void char_select_get_starting_zone_and_loc(R(CharSelect*) charSelect, uint16_t race, uint8_t class, uint8_t gender, bool isTrilogy,
        R(int*) zoneId, R(float*) x, R(float*) y, R(float*) z)
{
    R(lua_State*) L = charSelect->L;
    
    lua_getglobal(L, "char_select_get_starting_zone_and_loc");
    
    if (!lua_isfunction(L, -1))
        goto err_default;
    
    lua_pushinteger(L, race);
    lua_pushinteger(L, class);
    lua_pushinteger(L, gender);
    lua_pushboolean(L, isTrilogy);
    
    if (lua_sys_call_no_throw(B(charSelect), L, 4, 4))
    {
        *zoneId = lua_tointeger(L, -4);
        *x      = lua_tonumber(L, -3);
        *y      = lua_tonumber(L, -2);
        *z      = lua_tonumber(L, -1);
        
        lua_clear(L);
        return;
    }
    
err_default:
    lua_clear(L);
    
    *zoneId = 1;
    *x      = 0.0f;
    *y      = 0.0f;
    *z      = 0.0f;
}

#define make_ip(a, b, c, d) (((a << 0) & 0xff) | ((b << 8) & 0xff00) | ((c << 16) & 0xff0000) | ((d << 24) & 0xff000000))

int char_select_is_ip_address_local(uint32_t ip)
{
    uint32_t check;
    
    // Is it the loopback address 127.0.0.1?
    check = make_ip(127, 0, 0, 1);
    if (ip == check)
        goto ret_true;
    
    // Is it on the 192.168.x.x subnet?
    check = make_ip(192, 168, 0, 0);
    if ((ip & 0x0000ffff) == check)
        goto ret_true;
    
    // Is it on the 10.x.x.x subnet?
    check = make_ip(10, 0, 0, 0);
    if ((ip & 0x000000ff) == check)
        goto ret_true;
    
    // Is it on the 172.16.0.0 to 172.31.255.255 subnet (12-bit mask)?
    check = make_ip(172, 16, 0, 0);
    if ((ip & 0x0000f0ff) == check)
        goto ret_true;

    return false;
    
ret_true:
    return true;
}

#undef make_ip
