
#ifndef EQP_LOGIN_H
#define EQP_LOGIN_H

#include "define.h"
#include "eqp_core.h"
#include "eqp_log.h"
#include "eqp_clock.h"
#include "source_id.h"
#include "ipc_set.h"
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
    
    IpcSet          ipcSet;
    
    ServerList      serverList;
    ClientList      clientList;
    
    UdpSocket*      socket;
    TcpServer       tcpServer;
    LoginCrypto*    crypto;
};

void    login_init(Login* login, const char* ipcPath, const char* masterIpcPath, const char* logWriterIpcPath);
void    login_deinit(Login* login);
void    login_main_loop(Login* login);

int     login_is_ip_address_local(uint32_t ip);

#define login_get_crypto(login) ((login)->crypto)
#define login_server_list(login) (&(login)->serverList)
#define login_client_list(login) (&(login)->clientList)
#define login_tcp_server(login) (&(login)->tcpServer)

#endif//EQP_LOGIN_H
