
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
    AtomicMutex mutex;
    uint64_t    lastActivityTimestamp;
    uint64_t    creationTimestamp;
    pid_t       pid;
    ShmViewer   shmViewer;
    ShmCreator  shmCreator;
};

void        proc_init(R(ChildProcess*) proc);

void        proc_create_ipc_buffer(R(Master*) M, R(ChildProcess*) proc, R(const char*) path);
void        proc_start(R(ChildProcess*) proc, pid_t pid);
void        proc_shutdown(R(Master*) M, R(ChildProcess*) proc);
void        proc_kill(R(ChildProcess*) proc);

void        proc_ipc_send(R(Master*) M, R(ChildProcess*) proc, ServerOp opcode, uint32_t length, R(const void*) data);

void        proc_update_last_activity_time(R(ChildProcess*) proc);
uint64_t    proc_last_activity_time(R(ChildProcess*) proc);

#define     proc_ipc(proc) ((proc)->ipc)
#define     proc_creation_time(proc) ((proc)->creationTimestamp)
#define     proc_pid(proc) ((proc)->pid)
#define     proc_shm_path(proc) share_mem_path(&(proc)->shmCreator)

#endif//EQP_CHILD_PROCESS_H
