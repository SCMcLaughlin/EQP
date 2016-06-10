
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
#include "login_crypto.h"
#include "server_list.h"

STRUCT_DEFINE(Login)
{
    // Core MUST be the first member of this struct
    Core            core;
    
    ServerList      serverList;
    ShmViewer       shmViewerLogWriter;
    UdpSocket*      socket;
    LoginCrypto*    crypto;
};

void    login_init(R(Login*) login, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath);
void    login_deinit(R(Login*) login);
void    login_main_loop(R(Login*) login);

#define login_get_crypto(login) ((login)->crypto)
#define login_server_list(login) (&(login)->serverList)

#endif//EQP_LOGIN_H
