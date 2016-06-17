
#ifndef EQP_TCP_SERVER_H
#define EQP_TCP_SERVER_H

#include "define.h"
#include "netcode.h"
#include "eqp_array.h"
#include "eqp_log.h"
#include "tcp_client.h"

#define EQP_TCP_SERVER_COMPRESS_BUFFER_SIZE 2048

STRUCT_DECLARE(Login);

STRUCT_DEFINE(TcpServer)
{
    int     acceptFd;
    Login*  login;
    Array*  clients;
};

void    tcp_server_init(R(Login*) login, R(TcpServer*) server);
void    tcp_server_deinit(R(TcpServer*) server);

void    tcp_server_open(R(TcpServer*) server, uint16_t port);
void    tcp_server_close(R(TcpServer*) server);
void    tcp_server_accept_new_connections(R(TcpServer*) server);
void    tcp_server_recv(R(TcpServer*) server);
void    tcp_server_close_client(R(TcpServer*) server, R(TcpClient*) client);

void    tcp_server_send_client_login_request(R(TcpServer*) server, int loginServerIndex, uint32_t accountId);

#endif//EQP_TCP_SERVER_H
