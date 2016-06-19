
#ifndef EQP_LOGIN_H
#define EQP_LOGIN_H

#include "define.h"
#include "eqp_core.h"
#include "eqp_log.h"
#include "eqp_clock.h"
#include "source_id.h"
#include "server_op.h"
#include "ipc_buffer.h"
#include "share_mem.h"
#include "udp_socket.h"
#include "tcp_server.h"
#include "login_crypto.h"
#include "server_list.h"
#include "client_list.h"

#define EQP_LOGIN_PORT 5998
#define EQP_LOGIN_KEEP_ALIVE_DELAY_MILLISECONDS 5000

STRUCT_DEFINE(Login)
{
    // Core MUST be the first member of this struct
    Core            core;
    
    IpcBuffer*      ipc;
    IpcBuffer*      ipcMaster;
    ShmViewer       shmViewerSelf;
    ShmViewer       shmViewerMaster;
    ShmViewer       shmViewerLogWriter;
    
    ServerList      serverList;
    ClientList      clientList;
    
    UdpSocket*      socket;
    TcpServer       tcpServer;
    LoginCrypto*    crypto;
};

void    login_init(R(Login*) login, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath);
void    login_deinit(R(Login*) login);
void    login_main_loop(R(Login*) login);

#define login_get_crypto(login) ((login)->crypto)
#define login_server_list(login) (&(login)->serverList)
#define login_client_list(login) (&(login)->clientList)
#define login_tcp_server(login) (&(login)->tcpServer)

#endif//EQP_LOGIN_H
