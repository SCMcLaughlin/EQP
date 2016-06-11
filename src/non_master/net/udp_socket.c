
#include "udp_socket.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"

#define ERR_SOCKET "[udp_socket_open] socket() syscall failed"
#define ERR_NONBLOCK "[udp_socket_open] Setting non-blocking mode failed"
#define ERR_BIND "[udp_socket_open] bind() syscall failed"

UdpSocket* udp_socket_create(R(Basic*) basic)
{
    UdpSocket* sock = eqp_alloc_type(basic, UdpSocket);
    
    sock->socketFd  = INVALID_SOCKET;
    sock->basic     = basic;
    sock->clients   = array_create_type(basic, UdpClient);
    
    return sock;
}

void udp_socket_destroy(R(UdpSocket*) sock)
{
    udp_socket_close(sock);
    free(sock);
}

void udp_socket_open(R(UdpSocket*) sock, uint16_t port)
{
    IpAddress addr;
#ifdef EQP_WINDOWS
    unsigned long nonblock = 1;
#endif
    
    sock->socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sock->socketFd == INVALID_SOCKET)
        exception_throw_literal(sock->basic, ErrorNetwork, ERR_SOCKET);
    
    // Set non-blocking
#ifdef EQP_WINDOWS
    if (ioctlsocket(sock->socketFd, FIONBIO, &nonblock))
#else
    if (fcntl(sock->socketFd, F_SETFL, O_NONBLOCK))
#endif
        exception_throw_literal(sock->basic, ErrorNetwork, ERR_NONBLOCK);
    
    memset(&addr, 0, sizeof(IpAddress));
    
    addr.sin_family         = AF_INET;
    addr.sin_port           = toNetworkUint16(port);
    addr.sin_addr.s_addr    = toNetworkUint32(INADDR_ANY);
    
    if (bind(sock->socketFd, (struct sockaddr*)&addr, sizeof(IpAddress)))
        exception_throw_literal(sock->basic, ErrorNetwork, ERR_BIND);
    
    log_format(sock->basic, LogNetwork, "Listening for UDP packets on port %u", port);
}

void udp_socket_close(R(UdpSocket*) sock)
{
    if (sock->socketFd != INVALID_SOCKET)
    {
        closesocket(sock->socketFd);
        sock->socketFd = INVALID_SOCKET;
    }
}

static void udp_socket_handle_dead_client(R(UdpSocket*) sock, R(UdpClient*) cli, uint32_t index)
{
    udp_client_deinit(cli);
    if (array_swap_and_pop(sock->clients, index))
        protocol_handler_update_index(udp_client_handler(array_get_type(sock->clients, index, UdpClient)), index);
    printf("Destroyed UdpClient at index %u\n", index);
}

void udp_socket_recv(R(UdpSocket*) sock)
{
    IpAddress addr;
    socklen_t addrLen   = sizeof(IpAddress);
    int fd              = sock->socketFd;
    R(byte*) buffer     = sock->recvBuffer;
    
    for (;;)
    {
        UdpClient* array;
        UdpClient* client;
        uint32_t ip, i, n;
        uint16_t port;
        int len;

        len = recvfrom(fd, (char*)buffer, EQP_UDP_SOCKET_BUFFER_SIZE, 0, (struct sockaddr*)&addr, &addrLen);
        
        if (len <= 0)
        {
            int err = errno;
            if (err != EAGAIN)
                log_format(sock->basic, LogNetwork, "[udp_socket_recv] recvfrom() syscall failed, errno %i", err);
            return;
        }
        
        printf("Recv raw (%i):\n", len);
        for (i = 0; i < (uint32_t)len; i++) printf("%02x ", buffer[i]); printf("\n");
        
        // Do we already know about this client?
        client  = NULL;
        ip      = addr.sin_addr.s_addr;
        port    = addr.sin_port;
        
        array   = array_data_type(sock->clients, UdpClient);
        i       = 0;
        n       = array_count(sock->clients);
        
        while (i < n)
        {
            UdpClient* cli = &array[i];
            
            if (udp_client_is_dead(cli))
            {
                udp_socket_handle_dead_client(sock, cli, i);
                n--;
                continue;
            }
            
            if (udp_client_ip(cli) == ip && udp_client_port(cli) == port)
            {
                client = cli;
                goto found;
            }
            
            i++;
        }
        
        // If we reach here, this is a new client
        client = array_push_back_type(sock->basic, &sock->clients, UdpClient);
        udp_client_init(sock->basic, client, ip, port);
        protocol_handler_check_first_packet(sock, client, udp_client_handler(client), buffer, len, n);
        
    found:
        udp_client_update_last_recv_time(client);
        protocol_handler_recv(udp_client_handler(client), buffer, len);
    }
}

void udp_socket_check_timeouts(R(UdpSocket*) sock)
{
    UdpClient* array    = array_data_type(sock->clients, UdpClient);
    uint32_t n          = array_count(sock->clients);
    uint64_t time       = clock_milliseconds();
    uint32_t i          = 0;
    
    while (i < n)
    {
        R(UdpClient*) cli = &array[i];
        
        if (udp_client_is_dead(cli))
        {
            udp_socket_handle_dead_client(sock, cli, i);
            n--;
            continue;
        }
        
        if ((udp_client_last_recv_time(cli) + EQP_UDP_SOCKET_LINKDEAD_TIMEOUT_MILLISECONDS) < time)
        {
            udp_client_flag_as_dead(cli);
            client_on_disconnect(protocol_handler_client_object(udp_client_handler(cli)), true);
        }
        
        i++;
    }
}

void udp_socket_send(R(UdpSocket*) sock)
{
    UdpClient* array    = array_data_type(sock->clients, UdpClient);
    uint32_t n          = array_count(sock->clients);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        protocol_handler_send_queued(array[i].handler);
    }
}

void udp_socket_flag_client_as_dead_by_index(R(UdpSocket*) sock, uint32_t index)
{
    UdpClient* cli = array_get_type(sock->clients, index, UdpClient);
    
    if (cli)
        udp_client_flag_as_dead(cli);
}

#undef ERR_SOCKET
#undef ERR_NONBLOCK
#undef ERR_BIND
