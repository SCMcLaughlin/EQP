
#include "eqp_char_select.h"

void char_select_init(R(CharSelect*) charSelect, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath)
{
    (void)ipcPath;
    (void)masterIpcPath;
    
    charSelect->serverStatus        = EQP_CHAR_SELECT_SERVER_LOCKED;
    charSelect->serverPlayerCount   = 0;
    
    timer_pool_init(B(charSelect), &charSelect->timerPool);
    
    shm_viewer_init(&charSelect->shmViewerLogWriter);
    shm_viewer_open(B(charSelect), &charSelect->shmViewerLogWriter, logWriterIpcPath, sizeof(IpcBuffer));
    // Tell the log writer to open our log file
    ipc_buffer_write(B(charSelect), shm_viewer_memory_type(&charSelect->shmViewerLogWriter, IpcBuffer), ServerOp_LogOpen, EQP_SOURCE_ID_CHAR_SELECT, 0, NULL);
    
    core_init(C(charSelect), EQP_SOURCE_ID_CHAR_SELECT, shm_viewer_memory_type(&charSelect->shmViewerLogWriter, IpcBuffer));
    
    charSelect->L = lua_sys_open(B(charSelect));
    lua_sys_run_file(B(charSelect), charSelect->L, EQP_CHAR_SELECT_SCRIPT_CHAR_CREATE, 0);
    
    charSelect->socket = udp_socket_create(B(charSelect));
    udp_socket_open(charSelect->socket, EQP_CHAR_SELECT_PORT);
    
    charSelect->loginServerConnections = array_create_type(B(charSelect), TcpClient);
    
    timer_init(&charSelect->timerUnclaimedAuths, &charSelect->timerPool, EQP_CHAR_SELECT_UNCLAIMED_AUTHS_TIMEOUT,
        char_select_unclaimed_auths_timer_callback, charSelect, true);
    
    charSelect->unclaimedAuths  = array_create_type(B(charSelect), CharSelectAuth);
    charSelect->unauthedClients = array_create_type(B(charSelect), CharSelectClient*);
}

void char_select_deinit(R(CharSelect*) charSelect)
{
    core_deinit(C(charSelect));
    shm_viewer_close(&charSelect->shmViewerLogWriter);
    
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
}

void char_select_main_loop(R(CharSelect*) charSelect)
{
    R(UdpSocket*) socket = charSelect->socket;
    
    for (;;)
    {
        udp_socket_recv(socket);
        
        db_thread_execute_query_callbacks(core_db_thread(C(charSelect)));
        timer_pool_execute_callbacks(&charSelect->timerPool);
        
        udp_socket_send(socket);
        udp_socket_check_timeouts(socket);
        
        char_select_tcp_recv(charSelect);
        
        clock_sleep_milliseconds(50);
    }
}

void char_select_start_login_server_connections(R(CharSelect*) charSelect)
{
    R(lua_State*) L = charSelect->L;
    uint32_t n;
    uint32_t i;
    
    lua_sys_run_file(B(charSelect), L, EQP_LUA_SYS_LOGIN_CONFIG_SCRIPT, 1);
    
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
    R(CharSelectAuth*) array    = array_data_type(charSelect->unclaimedAuths, CharSelectAuth);
    uint32_t n                  = array_count(charSelect->unclaimedAuths);
    uint32_t i;
    
    // Check if we've already received an auth for this client
    for (i = 0; i < n; i++)
    {
        R(CharSelectAuth*) auth = &array[i];
        
        if (auth->accountId == accountId)
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
