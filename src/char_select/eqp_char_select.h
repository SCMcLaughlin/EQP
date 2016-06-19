
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
#include "timer.h"
#include "udp_socket.h"
#include "tcp_client.h"
#include "lua_sys.h"
#include "auth.h"
#include "char_select_client.h"

#define EQP_CHAR_SELECT_PORT            9000
#define EQP_CHAR_SELECT_SERVER_UP       0
#define EQP_CHAR_SELECT_SERVER_DOWN     -1
#define EQP_CHAR_SELECT_SERVER_LOCKED   -2
#define EQP_CHAR_SELECT_UNCLAIMED_AUTHS_TIMEOUT 30000

STRUCT_DEFINE(CharSelect)
{
    // Core MUST be the first member of this struct
    Core        core;
    
    int         serverStatus;
    int         serverPlayerCount;
    TimerPool   timerPool;
    ShmViewer   shmViewerLogWriter;
    lua_State*  L;
    UdpSocket*  socket;
    Array*      loginServerConnections;
    
    Timer       timerUnclaimedAuths;
    Array*      unclaimedAuths;
    Array*      unauthedClients;
};

void    char_select_init(R(CharSelect*) charSelect, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath);
void    char_select_deinit(R(CharSelect*) charSelect);
void    char_select_main_loop(R(CharSelect*) charSelect);

void    char_select_start_login_server_connections(R(CharSelect*) charSelect);
void    char_select_tcp_recv(R(CharSelect*) charSelect);

void    char_select_unclaimed_auths_timer_callback(R(Timer*) timer);
void    char_select_handle_client_auth(R(CharSelect*) charSelect, R(CharSelectAuth*) auth);
void    char_select_handle_unauthed_client(R(CharSelect*) charSelect, R(CharSelectClient*) client);
void    char_select_remove_client_from_unauthed_list(R(CharSelect*) charSelect, R(CharSelectClient*) client);

#define char_select_server_status(cs) ((cs)->serverStatus)
#define char_select_player_count(cs) ((cs)->serverPlayerCount)
#define char_select_timer_pool(cs) (&(cs)->timerPool)

#endif//EQP_CHAR_SELECT_H
