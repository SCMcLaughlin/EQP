
#ifndef EQP_TCP_CLIENT_H
#define EQP_TCP_CLIENT_H

#include "define.h"
#include "netcode.h"
#include "aligned.h"
#include "tcp_packet_structs.h"
#include "timer.h"
#include "exception.h"
#include "eqp_clock.h"
#include "eqp_string.h"
#include "database.h"
#include "query.h"

#define EQP_TCP_CLIENT_BUFFER_SIZE 1024
#define EQP_TCP_CLIENT_REMOTE_TIMEOUT_MILLISECONDS 60000

STRUCT_DECLARE(CharSelect);

STRUCT_DEFINE(LoginServerConfig)
{
    uint32_t    index;
    CharSelect* charSelect;
    String*     longName;
    String*     shortName;
    String*     host;
    String*     port;
    String*     username;
    String*     password;
    String*     remoteIp;
    String*     localIp;
};

STRUCT_DEFINE(TcpClient)
{
    int                 socketFd;
    int16_t             buffered;
    int16_t             readLength;
    byte*               recvBuf;
    LoginServerConfig*  config;
    CharSelect*         charSelect;
    uint64_t            lastRemoteTime;
    int                 isLocked;
    Timer*              timer;  // For reconnection when not connected, or status updates when connected
};

void    tcp_client_init(CharSelect* charSelect, TcpClient* client, LoginServerConfig* config, int locked);
void    tcp_client_deinit(TcpClient* client);

void    tcp_client_start_connect_cycle(TcpClient* client, int immediate);
void    tcp_client_restart_connection(TcpClient* client);
void    tcp_client_handle_packet(TcpClient* client);

#define tcp_client_fd(cli) ((cli)->socketFd)
#define tcp_client_buffered(cli) ((cli)->buffered)
#define tcp_client_read_length(cli) ((cli)->readLength)
#define tcp_client_recv_buffer(cli) ((cli)->recvBuf)

#define tcp_client_set_buffered(cli, n) ((cli)->buffered = (n))
#define tcp_client_set_read_length(cli, n) ((cli)->readLength = (n))

#endif//EQP_TCP_CLIENT_H
