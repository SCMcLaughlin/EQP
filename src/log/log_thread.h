
#ifndef EQP_LOG_THREAD_H
#define EQP_LOG_THREAD_H

#include "define.h"
#include "eqp_thread.h"
#include "eqp_basic.h"
#include "eqp_array.h"
#include "atomic_mutex.h"
#include "ipc_buffer.h"
#include "server_op.h"
#include "source_id.h"
#include "zone_id.h"

STRUCT_DEFINE(LogThread)
{
    Thread      thread;
    AtomicMutex inQueueMutex;
    Array*      inQueue;
    Array*      writeQueue;
    Array*      logFiles;
};

void    log_thread_init(Basic* basic, LogThread* logThread);
void    log_thread_deinit(LogThread* logThread);
void    log_thread_main_loop(Thread* thread);
void    log_thread_post_message(Basic* basic, LogThread* logThread, IpcPacket* packet);
#define log_thread_start(basic, thread) thread_start_and_detach((basic), T(thread), log_thread_main_loop)

#endif//EQP_LOG_THREAD_H
