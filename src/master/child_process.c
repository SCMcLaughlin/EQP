
#include "child_process.h"
#include "eqp_master.h"

void proc_init(R(ChildProcess*) proc)
{
    proc->ipc                   = NULL;
    atomic_mutex_init(&proc->mutex);
    proc->lastActivityTimestamp = 0;
    proc->creationTimestamp     = 0;
    proc->pid                   = 0;
    shm_viewer_init(&proc->shmViewer);
}

static void proc_deinit(R(ChildProcess*) proc)
{
    if (proc->ipc)
    {
        proc->ipc = NULL;
        proc->pid = 0;
        share_mem_destroy(&proc->shmCreator, &proc->shmViewer);
    }
}

void proc_create_ipc_buffer(R(Master*) M, R(ChildProcess*) proc, R(const char*) path)
{
    atomic_mutex_lock(&proc->mutex);
    ipc_buffer_shm_create_init(B(M), &proc->ipc, &proc->shmCreator, &proc->shmViewer, path);
    atomic_mutex_unlock(&proc->mutex);
}

void proc_start(R(ChildProcess*) proc, pid_t pid)
{
    uint32_t time = clock_milliseconds();
    
    atomic_mutex_lock(&proc->mutex);
    
    proc->pid                   = pid;
    proc->lastActivityTimestamp = time;
    proc->creationTimestamp     = time;
    
    atomic_mutex_unlock(&proc->mutex);
}

void proc_shutdown(R(Master*) M, R(ChildProcess*) proc)
{
    atomic_mutex_lock(&proc->mutex);
    
    if (proc->ipc)
    {
        ipc_buffer_write(B(M), proc->ipc, ServerOpShutdown, EQP_SOURCE_ID_MASTER, 0, NULL);
        proc_deinit(proc);
    }
    
    atomic_mutex_unlock(&proc->mutex);
}

void proc_kill(R(ChildProcess*) proc)
{
    char command[256];
    
    atomic_mutex_lock(&proc->mutex);
    
    snprintf(command, sizeof(command), "kill -s 9 %i", proc->pid);
    system(command);
    
    proc_deinit(proc);
    
    atomic_mutex_unlock(&proc->mutex);
}

void proc_ipc_send(R(Master*) M, R(ChildProcess*) proc, ServerOp opcode, uint32_t length, R(const void*) data)
{
    if (proc->ipc)
        ipc_buffer_write(B(M), proc->ipc, opcode, EQP_SOURCE_ID_MASTER, length, data);
}

void proc_update_last_activity_time(R(ChildProcess*) proc)
{
    atomic_mutex_lock(&proc->mutex);
    proc->lastActivityTimestamp = clock_milliseconds();
    atomic_mutex_unlock(&proc->mutex);
}

uint64_t proc_last_activity_time(R(ChildProcess*) proc)
{
    uint64_t value;
    atomic_mutex_lock(&proc->mutex);
    value = proc->lastActivityTimestamp;
    atomic_mutex_unlock(&proc->mutex);
    return value;
}
