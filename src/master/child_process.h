
#ifndef EQP_CHILD_PROCESS_H
#define EQP_CHILD_PROCESS_H

#include "define.h"
#include "atomic_mutex.h"
#include "ipc_buffer.h"
#include "share_mem.h"
#include "source_id.h"
#include "server_op.h"

STRUCT_DECLARE(Master);

STRUCT_DEFINE(ChildProcess)
{
    IpcBuffer*  ipc;
    uint64_t    lastActivityTimestamp;
    uint64_t    creationTimestamp;
    pid_t       pid;
    ShmViewer   shmViewer;
    ShmCreator  shmCreator;
};

void        proc_init(ChildProcess* proc);
void        proc_deinit(ChildProcess* proc);

void        proc_create_ipc_buffer(Basic* basic, ChildProcess* proc, const char* path);
void        proc_open_ipc_buffer(Basic* basic, ChildProcess* proc, const char* path);
void        proc_start(ChildProcess* proc, pid_t pid);
void        proc_shutdown(Master* M, ChildProcess* proc);
void        proc_kill(ChildProcess* proc);

void        proc_ipc_send(Basic* basic, ChildProcess* proc, ServerOp opcode, int sourceId, uint32_t length, const void* data);

void        proc_update_last_activity_time(ChildProcess* proc);
uint64_t    proc_last_activity_time(ChildProcess* proc);

#define     proc_ipc(proc) ((proc)->ipc)
#define     proc_creation_time(proc) ((proc)->creationTimestamp)
#define     proc_pid(proc) ((proc)->pid)
#define     proc_shm_path(proc) share_mem_path(&(proc)->shmCreator)

#endif//EQP_CHILD_PROCESS_H
