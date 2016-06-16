
#include "tcp_client.h"
#include "eqp_char_select.h"
#include "eqp_alloc.h"

#define RECONNECT_MILLISECONDS      15000
#define STATUS_UPDATE_MILLISECONDS  15000

void tcp_client_init(R(CharSelect*) charSelect, R(TcpClient*) client, R(LoginServerConfig*) config)
{
    client->socketFd    = INVALID_SOCKET;
    client->buffered    = 0;
    client->readLength  = sizeof(TcpPacketHeader);
    client->recvBuf     = eqp_alloc_type_bytes(B(charSelect), EQP_TCP_CLIENT_BUFFER_SIZE, byte);
    client->config      = config;
    
    timer_init(&client->timer, char_select_timer_pool(charSelect), RECONNECT_MILLISECONDS, NULL, charSelect, false);
}

void tcp_client_deinit(R(TcpClient*) client)
{
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

static void tcp_client_status_update_callback(R(Timer*) timer)
{
    R(CharSelect*) charSelect = timer_userdata_type(timer, CharSelect);
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
        exception_throw(B(charSelect), ErrorNetwork);
    
    client->socketFd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    
    if (client->socketFd == INVALID_SOCKET)
        exception_throw(B(charSelect), ErrorNetwork);
    
    // Set reuseaddr
    if (setsockopt(client->socketFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)))
        exception_throw(B(charSelect), ErrorNetwork);
    
    // Set non-blocking
#ifdef EQP_WINDOWS
    if (ioctlsocket(client->socketFd, FIONBIO, &nonblock))
#else
    if (fcntl(client->socketFd, F_SETFL, O_NONBLOCK))
#endif
        exception_throw(B(charSelect), ErrorNetwork);
    
    if (connect(client->socketFd, result->ai_addr, result->ai_addrlen))
        exception_throw(B(charSelect), ErrorNetwork);
    
    // We are connected; send server info packet to the login server
    client->lastRemoteTime = clock_milliseconds();
    
    memset(&send, 0, sizeof(Tcp_NewLoginServerSend));
    
    aligned_init(B(charSelect), w, &send, sizeof(Tcp_NewLoginServerSend));
    
    // opcode
    aligned_write_uint16(w, TcpOp_NewLoginServer);
    // length
    aligned_write_uint16(w, sizeof(Tcp_NewLoginServerSend));
    // longName
    aligned_write_snprintf_full_advance(w, sizeof_field(Tcp_NewLoginServerSend, data) + sizeof_field(Tcp_NewLoginServer, longName), "%s", string_data(config->longName));
    // shortName
    aligned_write_snprintf_full_advance(w, sizeof_field(Tcp_NewLoginServerSend, data) + sizeof_field(Tcp_NewLoginServer, shortName), "%s", string_data(config->shortName));
    // remoteAddress
    aligned_advance(w, sizeof_field(Tcp_NewLoginServerSend, data) + sizeof_field(Tcp_NewLoginServer, remoteAddress));
    //finish this
    /*
    char    localAddress[125];      // DNS address of the server
    char    account[31];            // account name for the worldserver
    char    password[31];           // password for the name
    char    protocolversion[25];    // Major protocol version number
    char    serverversion[64];      // minor server software version number
    uint8_t servertype;             // 0=world, 1=chat, 2=login, 3=MeshLogin
    */
    //tcp_client_send(charSelect, client, &send, sizeof(Tcp_NewLoginServerSend));
    
    // Start status update cycle
    timer_set_period_milliseconds(timer, STATUS_UPDATE_MILLISECONDS);
    timer_set_callback(timer, tcp_client_status_update_callback);
    timer_restart(timer);
    timer_execute_callback(timer);
}

static void tcp_client_reconnect_callback(R(Timer*) timer)
{
    ExceptionScope exScope;
    R(CharSelect*) volatile charSelect  = timer_userdata_type(timer, CharSelect);
    R(TcpClient*) volatile client       = char_select_tcp_client(charSelect);
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

void tcp_client_recv(R(TcpClient*) client)
{
    
}

#undef RECONNECT_MILLISECONDS
#undef STATUS_UPDATE_MILLISECONDS
