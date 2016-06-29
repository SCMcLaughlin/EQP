
#include "master_ipc.h"
#include "eqp_master.h"

void master_ipc_thread_init(Master* M, MasterIpcThread* ipcThread)
{
    memset(ipcThread, 0, sizeof(MasterIpcThread));
    
    atomic_mutex_init(&ipcThread->mutexShutdown);
    atomic_mutex_lock(&ipcThread->mutexShutdown);
    
    ipcThread->master   = M;
    ipcThread->db       = core_db(C(M));
}

static int master_ipc_thread_handle_packet(MasterIpcThread* ipcThread, Master* M, IpcPacket* packet)
{
    ServerOp opcode = ipc_packet_opcode(packet);
    int sourceId    = ipc_packet_source_id(packet);
    int ret         = false;
    ZoneCluster* zc = NULL;
    ChildProcess* proc;
    
    master_ipc_thread_lock(ipcThread);
    
    if (sourceId == EQP_SOURCE_ID_CONSOLE)
    {
        ret = console_receive(ipcThread, M, NULL, EQP_SOURCE_ID_CONSOLE, packet);
        goto finish;
    }
    
    if (sourceId >= EQP_SOURCE_ID_PROCESS_OFFSET)
    {
        proc = master_get_child_process(M, sourceId);
    }
    else
    {
        zc      = client_mgr_get_zone_cluster(&ipcThread->clientMgr, sourceId);
        proc    = zone_cluster_proc(zc);
    }
    
    if (proc == NULL)
    {
        log_format(B(ipcThread), LogError, "Received IPC request from unknown or inactive process, sourceId: %i", sourceId);
        goto finish;
    }
    
    if (proc->ipc == NULL)
    {
        log_format(B(ipcThread), LogError, "Received IPC request from process with NULL IpcBuffer, sourceId: %i, pid: %u", sourceId, proc->pid);
        goto finish;
    }
    
    proc_update_last_activity_time(proc);
    
    switch (opcode)
    {
    case ServerOp_KeepAlive:
        break;
    
    case ServerOp_ConsoleMessage:
        ret = console_receive(ipcThread, M, proc, sourceId, packet);
        break;
    
    case ServerOp_ClientZoning:
        client_mgr_handle_zone_in_from_char_select(&ipcThread->clientMgr, packet);
        break;
    
    default:
        log_format(B(ipcThread), LogError, "Received unexpected ServerOp: %u", opcode);
        break;
    }
    
finish:
    master_ipc_thread_unlock(ipcThread);
    return ret;
}

static void master_ipc_loop(MasterIpcThread* ipcThread)
{
    Master* M       = master_ipc_thread_master(ipcThread);
    IpcBuffer* ipc  = master_ipc_thread_ipc_buffer(ipcThread);
    
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
                return;
        }
    }
}

void master_ipc_thread_main_loop(Thread* thread)
{
    MasterIpcThread* ipcThread  = (MasterIpcThread*)thread;
    lua_State* L                = lua_sys_open(B(ipcThread));
    ipcThread->L                = L;
    
    lua_sys_run_file(B(ipcThread), L, EQP_MASTER_IPC_SCRIPT, 0);
    
    client_mgr_init(ipcThread, &ipcThread->clientMgr, L);
    
    master_ipc_loop(ipcThread);
    
    client_mgr_deinit(&ipcThread->clientMgr);
    lua_close(L);
    atomic_mutex_unlock(&ipcThread->mutexShutdown);
}
