
#ifndef EQP_LOG_WRITER_H
#define EQP_LOG_WRITER_H

#include "define.h"
#include "eqp_basic.h"
#include "log_thread.h"
#include "ipc_buffer.h"
#include "share_mem.h"
#include "eqp_string.h"

STRUCT_DEFINE(LogWriter)
{
    // Basic must be the first member of this struct
    Basic       basicState;
    LogThread   thread;
    ShmViewer   shmViewer;
};

void    log_writer_init(R(LogWriter*) logWriter, R(const char*) ipcPath);
void    log_writer_deinit(R(LogWriter*) logWriter);
void    log_writer_main_loop(R(LogWriter*) logWriter);

#endif//EQP_LOG_WRITER_H
