
#ifndef EQP_CHAR_SELECT_H
#define EQP_CHAR_SELECT_H

#include "define.h"
#include "eqp_core.h"
#include "eqp_log.h"
#include "eqp_clock.h"
#include "source_id.h"
#include "server_op.h"
#include "ipc_buffer.h"
#include "share_mem.h"
#include "timer_pool.h"
#include "udp_socket.h"

STRUCT_DEFINE(CharSelect)
{
    // Core MUST be the first member of this struct
    Core        core;
    
    TimerPool   timerPool;
    ShmViewer   shmViewerLogWriter;
    UdpSocket*  socket;
    //TcpClient   tcpClient;
};

void    char_select_init(R(CharSelect*) charSelect, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath);
void    char_select_deinit(R(CharSelect*) charSelect);
void    char_select_main_loop(R(CharSelect*) charSelect);

#endif//EQP_CHAR_SELECT_H
