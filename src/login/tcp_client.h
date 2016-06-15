
#ifndef EQP_TCP_CLIENT_H
#define EQP_TCP_CLIENT_H

#include "define.h"
#include "netcode.h"
#include "aligned.h"
#include "tcp_packet_structs.h"
#include "server_list.h"
#include "client_list.h"
#include "login_client.h"

#define EQP_TCP_CLIENT_BUFFER_SIZE 1024

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(Login);

STRUCT_DEFINE(TcpClient)
{
    int         socketFd;
    int16_t     buffered;
    int16_t     readLength;
    int         loginServerIndex;
    IpAddress   address;
    byte*       recvBuf;
};

void    tcp_client_init(R(Basic*) basic, R(TcpClient*) client, int fd, R(IpAddress*) addr);
void    tcp_client_deinit(R(TcpClient*) client);

void    tcp_client_handle_packet(R(Login*) login, R(TcpClient*) client);
void    tcp_client_send_client_login_request(R(Login*) login, R(TcpClient*) client, uint32_t accountId);

#define tcp_client_fd(cli) ((cli)->socketFd)
#define tcp_client_buffered(cli) ((cli)->buffered)
#define tcp_client_read_length(cli) ((cli)->readLength)
#define tcp_client_has_login_server(cli) ((cli)->loginServerIndex != -1)
#define tcp_client_login_server_index(cli) ((cli)->loginServerIndex)
#define tcp_client_address(cli) ((cli)->address)
#define tcp_client_recv_buffer(cli) ((cli)->recvBuf)

#define tcp_client_set_buffered(cli, n) ((cli)->buffered = (n))
#define tcp_client_set_read_length(cli, n) ((cli)->readLength = (n))
#define tcp_client_set_login_server_index(cli, index) ((cli)->loginServerIndex = (index))

#endif//EQP_TCP_CLIENT_H
