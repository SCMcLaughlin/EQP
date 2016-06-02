
#ifndef _EQP_MASTER_H_
#define _EQP_MASTER_H_

#include "define.h"
#include "eqp_core.h"
#include "eqp_log.h"
#include "source_id.h"
#include "ipc_buffer.h"
#include "share_mem.h"

STRUCT_DEFINE(ChildProcess)
{
    IpcBuffer*  ipc;
    pid_t       pid;
    ShmViewer   shmViewer;
    ShmCreator  shmCreator;
};

STRUCT_DEFINE(Master)
{
    // Core MUST be the first member of this struct
    Core core;
    
    // Child processes and associated shared memory
    ChildProcess    procCharSelect;
    ChildProcess    procLogWriter;
    ChildProcess    procLogin;
};

void    master_init(R(Master*) M);
void    master_deinit(R(Master*) M);
void    master_shut_down_all_child_processes(R(Master*) M);

void    master_start_log_writer(R(Master*) M);
void    master_start_char_select(R(Master*) M);
void    master_start_login(R(Master*) M);

#endif//_EQP_MASTER_H_
