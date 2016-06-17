
#include "tcp_client.h"
#include "eqp_char_select.h"
#include "eqp_alloc.h"

#define RECONNECT_MILLISECONDS      15000
#define STATUS_UPDATE_MILLISECONDS  15000
#define PROTOCOL_VERSION            "0.3.10"
#define LOGIN_VERSION               "0.8.0"
#define SERVER_TYPE_WORLD           0

void tcp_client_init(R(CharSelect*) charSelect, R(TcpClient*) client, R(LoginServerConfig*) config)
{
    client->socketFd    = INVALID_SOCKET;
    client->buffered    = 0;
    client->readLength  = sizeof(TcpPacketHeader);
    client->recvBuf     = eqp_alloc_type_bytes(B(charSelect), EQP_TCP_CLIENT_BUFFER_SIZE, byte);
    client->config      = config;
    client->charSelect  = charSelect;
    
    timer_init(&client->timer, char_select_timer_pool(charSelect), RECONNECT_MILLISECONDS, NULL, client, false);
}

void tcp_client_deinit(R(TcpClient*) client)
{
    if (client->socketFd != INVALID_SOCKET)
    {
        closesocket(client->socketFd);
        client->socketFd = INVALID_SOCKET;
    }
    
    if (client->recvBuf)
    {
        free(client->recvBuf);
        client->recvBuf = NULL;
    }
    
    if (client->config)
    {
        R(LoginServerConfig*) c = client->config;
        
        if (c->longName)
            string_destroy(c->longName);
        
        if (c->shortName)
            string_destroy(c->shortName);
        
        if (c->host)
            string_destroy(c->host);
        
        if (c->port)
            string_destroy(c->port);
        
        if (c->username)
            string_destroy(c->username);
        
        if (c->password)
            string_destroy(c->password);
        
        free(c);
        client->config = NULL;
    }
    
    timer_deinit(&client->timer);
}

static void tcp_client_restart_connection(R(TcpClient*) client)
{
    closesocket(client->socketFd);
    client->socketFd = INVALID_SOCKET;
    tcp_client_start_connect_cycle(client);
}

static void tcp_client_send(R(CharSelect*) charSelect, R(TcpClient*) client, R(const void*) data, int size)
{
    int fd      = client->socketFd;
    int sent    = 0;
    
    for (;;)
    {
        int len = send(fd, ((byte*)data) + sent, size - sent, 0);
        
        if (len == -1)
        {
            int err = errno;
            if (err != EAGAIN && err != EWOULDBLOCK)
            {
                if (err == ECONNREFUSED)
                {
                    tcp_client_restart_connection(client);
                    return;
                }
                
                log_format(B(charSelect), LogNetwork, "[tcp_client_send] send() syscall failed, errno %i", err);
                break;
            }
        }
        
        sent += len;
        
        if (sent == size)
            break;
    }
}

static void tcp_client_status_update_callback(R(Timer*) timer)
{
    R(TcpClient*) client        = timer_userdata_type(timer, TcpClient);
    R(CharSelect*) charSelect   = client->charSelect;
    Aligned write;
    R(Aligned*) w = &write;
    Tcp_LoginServerStatusSend send;
    
    // Check for timeout
    if ((clock_milliseconds() - client->lastRemoteTime) >= EQP_TCP_CLIENT_REMOTE_TIMEOUT_MILLISECONDS)
    {
        tcp_client_restart_connection(client);
        return;
    }
    
    aligned_init(B(charSelect), w, &send, sizeof(Tcp_LoginServerStatusSend));
    
    // opcode
    aligned_write_uint16(w, TcpOp_LoginServerStatus);
    // length
    aligned_write_uint16(w, sizeof(Tcp_LoginServerStatusSend));
    // status
    aligned_write_int32(w, char_select_server_status(charSelect));
    // playerCount
    aligned_write_int32(w, char_select_player_count(charSelect));
    // zoneCount
    aligned_write_int32(w, 1); // We don't really care about this...
    
    tcp_client_send(charSelect, client, &send, sizeof(Tcp_LoginServerStatusSend));
}

static void tcp_client_get_local_address(R(TcpClient*) client, R(Aligned*) w, uint32_t len)
{
    IpAddress addr;
    socklen_t addrlen = sizeof(IpAddress);
    
    if (getsockname(client->socketFd, (struct sockaddr*)&addr, &addrlen))
    {
        int err = errno;
        log_format(B(client->charSelect), LogNetwork, "[tcp_client_get_local_address] getsockname() syscall failed, errno %i", err);
        aligned_advance(w, len);
    }
    else
    {
        uint32_t ip = addr.sin_addr.s_addr;
        aligned_write_snprintf_full_advance(w, len, "%u.%u.%u.%u", (ip >> 0) & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
    }
}

static void tcp_client_write_string_not_null(R(Aligned*) w, uint32_t len, R(String*) str)
{
    if (string_length(str) > 0)
        aligned_write_snprintf_full_advance(w, len, "%s", string_data(str));
    else
        aligned_advance(w, len);
}

static void tcp_client_do_connect(R(CharSelect*) charSelect, R(TcpClient*) client, R(Timer*) timer, struct addrinfo* result)
{
    R(LoginServerConfig*) config = client->config;
    Tcp_NewLoginServerSend send;
    Aligned write;
    R(Aligned*) w = &write;
    struct addrinfo hint;
    int opt = 1;
#ifdef EQP_WINDOWS
    unsigned long nonblock = 1;
#endif
    
    memset(&hint, 0, sizeof(struct addrinfo));
    
    hint.ai_family      = AF_INET;
    hint.ai_socktype    = SOCK_STREAM;
    
    if (getaddrinfo(string_data(config->host), string_data(config->port), &hint, &result))
        exception_throw_format(B(charSelect), ErrorNetwork, "[tcp_client_do_connect] getaddrinfo() syscall failed, errno %i", errno);
    
    client->socketFd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    
    if (client->socketFd == INVALID_SOCKET)
        exception_throw_format(B(charSelect), ErrorNetwork, "[tcp_client_do_connect] socket() syscall failed, errno %i", errno);
    
    // Set reuseaddr
    if (setsockopt(client->socketFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)))
        exception_throw_format(B(charSelect), ErrorNetwork, "[tcp_client_do_connect] setsockopt() syscall failed, errno %i", errno);
    
    // Set non-blocking
#ifdef EQP_WINDOWS
    if (ioctlsocket(client->socketFd, FIONBIO, &nonblock))
#else
    if (fcntl(client->socketFd, F_SETFL, O_NONBLOCK))
#endif
        exception_throw_format(B(charSelect), ErrorNetwork, "[tcp_client_do_connect] Setting non-blocking mode failed, errno %i", errno);
    
    // Set no-delay
    if (setsockopt(client->socketFd, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt)))
        exception_throw_format(B(charSelect), ErrorNetwork, "[tcp_client_do_connect] Setting no-delay mode failed, errno %i", errno);
    
    if (connect(client->socketFd, result->ai_addr, result->ai_addrlen))
    {
        int err = errno;
        if (err != EINPROGRESS)
            exception_throw_format(B(charSelect), ErrorNetwork, "[tcp_client_do_connect] connect() syscall failed, errno %i", errno);
    }
    
    // We are connected; send server info packet to the login server
    memset(&send, 0, sizeof(Tcp_NewLoginServerSend));
    
    aligned_init(B(charSelect), w, &send, sizeof(Tcp_NewLoginServerSend));
    
    // opcode
    aligned_write_uint16(w, TcpOp_NewLoginServer);
    // length
    aligned_write_uint16(w, sizeof(Tcp_NewLoginServerSend));
    // longName
    tcp_client_write_string_not_null(w, sizeof_field(Tcp_NewLoginServer, longName), config->longName);
    // shortName
    tcp_client_write_string_not_null(w, sizeof_field(Tcp_NewLoginServer, shortName), config->shortName);
    // remoteAddress
    tcp_client_write_string_not_null(w, sizeof_field(Tcp_NewLoginServer, remoteAddress), config->remoteIp);
    // localAddress
    if (string_length(config->localIp) > 0)
        tcp_client_write_string_not_null(w, sizeof_field(Tcp_NewLoginServer, localAddress), config->localIp);
    else
        tcp_client_get_local_address(client, w, sizeof_field(Tcp_NewLoginServer, localAddress));
    // account
    tcp_client_write_string_not_null(w, sizeof_field(Tcp_NewLoginServer, account), config->username);
    // password
    tcp_client_write_string_not_null(w, sizeof_field(Tcp_NewLoginServer, password), config->password);
    // protocolversion
    aligned_write_snprintf_full_advance(w, sizeof_field(Tcp_NewLoginServer, protocolversion), "%s", PROTOCOL_VERSION);
    // serverversion
    aligned_write_snprintf_full_advance(w, sizeof_field(Tcp_NewLoginServer, serverversion), "%s", LOGIN_VERSION);
    // servertype
    aligned_write_uint8(w, SERVER_TYPE_WORLD);

    tcp_client_send(charSelect, client, &send, sizeof(Tcp_NewLoginServerSend));
    
    // Start status update cycle
    client->lastRemoteTime = clock_milliseconds(); // Don't time out right away!
    
    timer_set_period_milliseconds(timer, STATUS_UPDATE_MILLISECONDS);
    timer_set_callback(timer, tcp_client_status_update_callback);
    timer_restart(timer);
    timer_execute_callback(timer);
}

static void tcp_client_reconnect_callback(R(Timer*) timer)
{
    ExceptionScope exScope;
    R(TcpClient*) volatile client       = timer_userdata_type(timer, TcpClient);
    R(CharSelect*) volatile charSelect  = client->charSelect;
    struct addrinfo* volatile result    = NULL;
    
    switch (exception_try(B(charSelect), &exScope))
    {
    case Try:
        tcp_client_do_connect(charSelect, client, timer, result);
        break;
    
    case Finally:
        if (result)
            freeaddrinfo(result);
        break;
    
    case ErrorNetwork:
        if (client->socketFd != INVALID_SOCKET)
        {
            closesocket(client->socketFd);
            client->socketFd = INVALID_SOCKET;
        }
        
        log_format(B(charSelect), LogNetwork, "%s", string_data(exception_get_message(B(charSelect))));
        printf("Error: %s\n", string_data(exception_get_message(B(charSelect))));
        
        exception_handled(B(charSelect));
        break;
    
    default:
        break;
    }
    
    exception_end_try_with_finally(B(charSelect));
}

void tcp_client_start_connect_cycle(R(TcpClient*) client)
{
    R(Timer*) timer = &client->timer;
    
    timer_set_period_milliseconds(timer, RECONNECT_MILLISECONDS);
    timer_set_callback(timer, tcp_client_reconnect_callback);
    timer_restart(timer);
    timer_execute_callback(timer);
}

static void tcp_client_handle_op_client_login_request(R(CharSelect*) charSelect, R(TcpClient*) client, R(Aligned*) a)
{
    Tcp_ClientLoginResponseSend send;
    Aligned write;
    R(Aligned*) w = &write;
    uint32_t accountId;
    uint32_t serverId;
    
    if (aligned_remaining(a) < sizeof(Tcp_ClientLoginRequest))
        return;
    
    accountId   = aligned_read_uint32(a);
    serverId    = aligned_read_uint32(a);
    
    //do db checking here...
    //put the reply packet in the db callback, too
    
    aligned_init(B(charSelect), w, &send, sizeof(Tcp_ClientLoginResponseSend));
    
    // opcode
    aligned_write_uint16(w, TcpOp_ClientLoginResponse);
    // length
    aligned_write_uint16(w, sizeof(Tcp_ClientLoginResponseSend));
    // accountId
    aligned_write_uint32(w, accountId);
    // serverId
    aligned_write_uint32(w, serverId); // This field seems pointless, login server can tell which server we are via our connection-oriented socket...
    // response
    aligned_write_int8(w, 1); // -3 = World Full, -2 = Banned, -1 = Suspended, 0 = Denied, 1 = Allowed
    // unused[2]
    aligned_write_zeroes(w, sizeof(uint32_t) * 2);
    
    tcp_client_send(charSelect, client, &send, sizeof(Tcp_ClientLoginResponseSend));
}

void tcp_client_handle_packet(R(TcpClient*) client)
{
    R(CharSelect*) charSelect = client->charSelect;
    Aligned aligned;
    R(Aligned*) a = &aligned;
    uint16_t opcode;
    
    client->lastRemoteTime = clock_milliseconds();
    
    aligned_init(B(charSelect), a, client->recvBuf, client->readLength);
    
    opcode = aligned_read_uint16(a);
    aligned_advance(a, sizeof(uint16_t)); // We already know the packet's data length
    
    printf("TCP packet, opcode: 0x%04x, length: %u\n", opcode, aligned_remaining(a));
    
    switch (opcode)
    {
    case TcpOp_ClientLoginRequest:
        tcp_client_handle_op_client_login_request(charSelect, client, a);
        break;
    /*case TcpOp_NewLoginServer:
        tcp_client_handle_op_new_login_server(login, client, a);
        break;
    
    case TcpOp_LoginServerStatus:
        tcp_client_handle_op_login_server_status(login, client, a);
        break;
    
    case TcpOp_ClientLoginResponse:
        tcp_client_handle_op_client_login_response(login, client, a);
        break;*/
    
    default:
        break;
    }
}

#undef RECONNECT_MILLISECONDS
#undef STATUS_UPDATE_MILLISECONDS
#undef PROTOCOL_VERSION
#undef LOGIN_VERSION
#undef SERVER_TYPE_WORLD
