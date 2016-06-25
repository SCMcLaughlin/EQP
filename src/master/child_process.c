
#include "child_process.h"
#include "eqp_master.h"

void proc_init(R(ChildProcess*) proc)
{
    proc->ipc                   = NULL;
    proc->lastActivityTimestamp = 0;
    proc->creationTimestamp     = 0;
    proc->pid                   = 0;
    shm_viewer_init(&proc->shmViewer);
    shm_creator_init(&proc->shmCreator);
}

void proc_deinit(R(ChildProcess*) proc)
{
    if (proc->ipc)
    {
        proc->ipc = NULL;
        proc->pid = 0;
        share_mem_destroy(&proc->shmCreator, &proc->shmViewer);
    }
}

void proc_create_ipc_buffer(R(Basic*) basic, R(ChildProcess*) proc, R(const char*) path)
{
    ipc_buffer_shm_create_init(basic, &proc->ipc, &proc->shmCreator, &proc->shmViewer, path);
}

void proc_open_ipc_buffer(R(Basic*) basic, R(ChildProcess*) proc, R(const char*) path)
{
    shm_viewer_open(basic, &proc->shmViewer, path, sizeof(IpcBuffer));
    proc->ipc = shm_viewer_memory_type(&proc->shmViewer, IpcBuffer);
}

void proc_start(R(ChildProcess*) proc, pid_t pid)
{
    uint32_t time = clock_milliseconds();
    
    proc->pid                   = pid;
    proc->lastActivityTimestamp = time;
    proc->creationTimestamp     = time;
}

void proc_shutdown(R(Master*) M, R(ChildProcess*) proc)
{
    if (proc->ipc)
    {
        ipc_buffer_write(B(M), proc->ipc, ServerOp_Shutdown, EQP_SOURCE_ID_MASTER, 0, NULL);
        proc_deinit(proc);
    }
}

void proc_kill(R(ChildProcess*) proc)
{
    char command[256];
    
    snprintf(command, sizeof(command), "kill -s 9 %i", proc->pid);
    system(command);
    
    proc_deinit(proc);
}

void proc_ipc_send(R(Master*) M, R(ChildProcess*) proc, ServerOp opcode, uint32_t length, R(const void*) data)
{
    if (proc->ipc)
        ipc_buffer_write(B(M), proc->ipc, opcode, EQP_SOURCE_ID_MASTER, length, data);
}

void proc_update_last_activity_time(R(ChildProcess*) proc)
{
    proc->lastActivityTimestamp = clock_milliseconds();
}

uint64_t proc_last_activity_time(R(ChildProcess*) proc)
{
    return proc->lastActivityTimestamp;
}
