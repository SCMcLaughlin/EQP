
#include "tcp_server.h"
#include "eqp_login.h"

#define ERR_SOCKET "[tcp_server_open] socket() syscall failed"
#define ERR_REUSEADDR "[tcp_server_open] Setting reuse-addr mode failed"
#define ERR_NONBLOCK "[tcp_server_open] Setting non-blocking mode failed"
#define ERR_BIND "[tcp_server_open] bind() syscall failed"
#define ERR_LISTEN "[tcp_server_open] listen() syscall failed"

void tcp_server_init(R(Login*) login, R(TcpServer*) server)
{
    server->acceptFd    = INVALID_SOCKET;
    server->login       = login;
    server->clients     = array_create_type(B(login), TcpClient);
}

void tcp_server_deinit(R(TcpServer*) server)
{
    tcp_server_close(server);
}

void tcp_server_open(R(TcpServer*) server, uint16_t port)
{
    IpAddress addr;
    int opt;
#ifdef EQP_WINDOWS
    unsigned long nonblock = 1;
#endif
    
    server->acceptFd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (server->acceptFd == INVALID_SOCKET)
        exception_throw_literal(B(server->login), ErrorNetwork, ERR_SOCKET);
    
    // Set reuseaddr
    opt = 1;
    if (setsockopt(server->acceptFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)))
        exception_throw_literal(B(server->login), ErrorNetwork, ERR_REUSEADDR);
    
    // Set non-blocking
#ifdef EQP_WINDOWS
    if (ioctlsocket(server->acceptFd, FIONBIO, &nonblock))
#else
    if (fcntl(server->acceptFd, F_SETFL, O_NONBLOCK))
#endif
        exception_throw_literal(B(server->login), ErrorNetwork, ERR_NONBLOCK);
    
    memset(&addr, 0, sizeof(IpAddress));
    
    addr.sin_family         = AF_INET;
    addr.sin_port           = toNetworkUint16(port);
    addr.sin_addr.s_addr    = toNetworkUint32(INADDR_ANY);
    
    if (bind(server->acceptFd, (struct sockaddr*)&addr, sizeof(IpAddress)))
        exception_throw_literal(B(server->login), ErrorNetwork, ERR_BIND);
    
    if (listen(server->acceptFd, SOMAXCONN))
        exception_throw_literal(B(server->login), ErrorNetwork, ERR_LISTEN);
    
    log_format(B(server->login), LogNetwork, "Listening for TCP connections on port %u", port);
}

void tcp_server_close(R(TcpServer*) server)
{
    if (server->acceptFd != INVALID_SOCKET)
    {
        closesocket(server->acceptFd);
        server->acceptFd = INVALID_SOCKET;
    }
    
    if (server->clients)
    {
        TcpClient* array    = array_data_type(server->clients, TcpClient);
        uint32_t n          = array_count(server->clients);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            tcp_client_deinit(&array[i]);
        }
        
        array_destroy(server->clients);
        server->clients = NULL;
    }
}

void tcp_server_accept_new_connections(R(TcpServer*) server)
{
    IpAddress addr;
    socklen_t addrLen   = sizeof(IpAddress);
    int acceptFd        = server->acceptFd;
#ifdef EQP_WINDOWS
    unsigned long nonblock = 1;
#endif
    
    for (;;)
    {
        TcpClient* client;
        uint32_t ip;
        int fd = accept(acceptFd, (struct sockaddr*)&addr, &addrLen);
        
        if (fd == -1)
        {
            int err = errno;
            if (err != EAGAIN && err != EWOULDBLOCK)
                log_format(B(server->login), LogNetwork, "[tcp_server_accept_new_connections] accept() syscall failed, errno %i", err);
            return;
        }
        
        // Set non-blocking
#ifdef EQP_WINDOWS
        if (ioctlsocket(fd, FIONBIO, &nonblock))
#else
        if (fcntl(fd, F_SETFL, O_NONBLOCK))
#endif
        {
            log_format(B(server->login), LogNetwork, "[tcp_server_accept_new_connections] Setting non-blocking mode failed");
            closesocket(fd);
            continue;
        }
        
        ip = addr.sin_addr.s_addr;
        
        log_format(B(server->login), LogNetwork, "[tcp_server_accept_new_connections] New server connection from %u.%u.%u.%u:%u",
            (ip >> 0) & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff, addr.sin_port);
        
        client = array_push_back(B(server->login), &server->clients, NULL);
        tcp_client_init(B(server->login), client, fd, &addr);
    }
}

static void tcp_server_handle_closed_client(R(TcpServer*) server, R(TcpClient*) cli, uint32_t index)
{
    printf("Remote end disconnected\n");
    if (tcp_client_has_login_server(cli))
        server_list_remove_by_index(login_server_list(server->login), tcp_client_login_server_index(cli));
    
    tcp_client_deinit(cli);
    
    if (array_swap_and_pop(server->clients, index))
        tcp_client_set_login_server_index(array_back_type(server->clients, TcpClient), index);
}

void tcp_server_recv(R(TcpServer*) server)
{
    R(Login*) login     = server->login;
    R(TcpClient*) array = array_data_type(server->clients, TcpClient);
    uint32_t n          = array_count(server->clients);
    uint32_t i          = 0;
    
    while (i < n)
    {
        R(TcpClient*) cli   = &array[i];
        int fd              = tcp_client_fd(cli);
        int buffered        = tcp_client_buffered(cli);
        int readLength      = tcp_client_read_length(cli);
        R(byte*) recvBuf    = tcp_client_recv_buffer(cli);
        int len;
        
    redo:
        len = recv(fd, recvBuf + buffered, readLength - buffered, 0);
        
        if (len == -1)
        {
            int err = errno;
            if (err != EAGAIN && err != EWOULDBLOCK)
                log_format(B(login), LogNetwork, "[tcp_server_recv] recv() syscall failed, errno: %i", err);
            
            goto increment;
        }
        
        if (len == 0)
        {
            // Remote end has closed the connection
            tcp_server_handle_closed_client(server, cli, i);
            n--;
            continue;
        }
        
        buffered += len;
        
        if (buffered == readLength)
        {
            if (buffered == sizeof(TcpPacketHeader))
            {
                readLength = *(uint16_t*)(&recvBuf[2]); // Aligned read
                tcp_client_set_read_length(cli, readLength);
            }
            else
            {
                // We have a complete packet to handle
                tcp_client_handle_packet(login, cli);
                
                buffered    = 0;
                readLength  = sizeof(TcpPacketHeader);
                tcp_client_set_buffered(cli, 0);
                tcp_client_set_read_length(cli, sizeof(TcpPacketHeader));
            }
            
            goto redo;
        }
        
        tcp_client_set_buffered(cli, buffered);
        
    increment:
        i++;
    }
}

void tcp_server_send_client_login_request(R(TcpServer*) server, int loginServerIndex, uint32_t accountId)
{
    R(TcpClient*) array = array_data_type(server->clients, TcpClient);
    uint32_t n          = array_count(server->clients);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        R(TcpClient*) cli = &array[i];
        
        if (tcp_client_login_server_index(cli) == loginServerIndex)
        {
            tcp_client_send_client_login_request(server->login, cli, accountId);
            return;
        }
    }
}

#undef ERR_SOCKET
#undef ERR_REUSERADDR
#undef ERR_NONBLOCK
#undef ERR_BIND
#undef ERR_LISTEN
