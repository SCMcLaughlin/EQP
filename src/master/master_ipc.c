
#include "master_ipc.h"
#include "eqp_master.h"

void master_ipc_thread_init(R(Master*) M, R(MasterIpcThread*) ipcThread)
{
    memset(ipcThread, 0, sizeof(MasterIpcThread));
    
    atomic_mutex_init(&ipcThread->mutexShutdown);
    atomic_mutex_lock(&ipcThread->mutexShutdown);
    
    ipcThread->master = M;
}

static int master_ipc_thread_handle_packet(R(MasterIpcThread*) ipcThread, R(Master*) M, R(IpcPacket*) packet)
{
    ServerOp opcode     = ipc_packet_opcode(packet);
    int sourceId        = ipc_packet_source_id(packet);
    ChildProcess* proc;
    
    if (sourceId == EQP_SOURCE_ID_CONSOLE)
        return console_receive(ipcThread, M, NULL, EQP_SOURCE_ID_CONSOLE, packet);
    
    proc = master_get_child_process(M, sourceId);
    
    if (proc == NULL)
    {
        log_format(B(ipcThread), LogError, "Received IPC request from unknown or inactive process, sourceId: %i", sourceId);
        return false;
    }
    
    if (proc->ipc == NULL)
    {
        log_format(B(ipcThread), LogError, "Received IPC request from process with NULL IpcBuffer, sourceId: %i, pid: %u", sourceId, proc->pid);
        return false;
    }
    
    proc_update_last_activity_time(proc);
    
    switch (opcode)
    {
    case ServerOp_KeepAlive:
        break;
    
    case ServerOp_ConsoleMessage:
        return console_receive(ipcThread, M, proc, sourceId, packet);
    
    default:
        log_format(B(ipcThread), LogError, "Received unexpected ServerOp: %u", opcode);
        break;
    }
    
    return false;
}

static void master_ipc_loop(R(MasterIpcThread*) ipcThread)
{
    R(Master*) M        = master_ipc_thread_master(ipcThread);
    R(IpcBuffer*) ipc   = master_ipc_thread_ipc_buffer(ipcThread);
    
    for (;;)
    {
        // Block until we receive an IPC packet
        ipc_buffer_wait(B(ipcThread), ipc);
        
        for (;;)
        {
            IpcPacket packet;
            int done;
            
            if (!ipc_buffer_read(B(ipcThread), ipc, &packet))
                break;
            
            done = master_ipc_thread_handle_packet(ipcThread, M, &packet);
            
            ipc_packet_deinit(&packet);
            
            if (done)
                break;
        }
    }
}

void master_ipc_thread_main_loop(R(Thread*) thread)
{
    R(MasterIpcThread*) ipcThread   = (MasterIpcThread*)thread;
    lua_State* L                    = lua_sys_open(B(ipcThread));
    ipcThread->L                    = L;
    
    lua_sys_run_file(B(ipcThread), L, EQP_MASTER_IPC_SCRIPT, 0);
    
    master_ipc_loop(ipcThread);
    
    atomic_mutex_unlock(&ipcThread->mutexShutdown);
    lua_close(L);
}
