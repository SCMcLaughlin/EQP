
#ifndef EQP_CHAR_SELECT_H
#define EQP_CHAR_SELECT_H

#include "define.h"
#include "eqp_core.h"
#include "eqp_log.h"
#include "eqp_clock.h"
#include "eqp_array.h"
#include "source_id.h"
#include "server_op.h"
#include "ipc_buffer.h"
#include "share_mem.h"
#include "timer_pool.h"
#include "udp_socket.h"
#include "tcp_client.h"
#include "lua_sys.h"

#define EQP_CHAR_SELECT_PORT 9000

STRUCT_DEFINE(CharSelect)
{
    // Core MUST be the first member of this struct
    Core        core;
    
    TimerPool   timerPool;
    ShmViewer   shmViewerLogWriter;
    lua_State*  L;
    UdpSocket*  socket;
    Array*      loginServerConnections;
    TcpClient   tcpClient;  //fixme: arrayize to allow arbitrary numbers of login servers
};

void    char_select_init(R(CharSelect*) charSelect, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath);
void    char_select_deinit(R(CharSelect*) charSelect);
void    char_select_main_loop(R(CharSelect*) charSelect);

void    char_select_start_login_server_connections(R(CharSelect*) charSelect);

#define char_select_timer_pool(cs) (&(cs)->timerPool)
#define char_select_tcp_client(cs) (&(cs)->tcpClient)

#endif//EQP_CHAR_SELECT_H
