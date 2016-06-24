
#include "ipc_set.h"
#include "eqp_basic.h"

void ipc_set_open(R(Basic*) basic, R(IpcSet*) ipcSet, int sourceId, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath)
{
    ipcSet->keepAliveTimestamp = 0;
    
    // Open our IPC buffer
    shm_viewer_init(&ipcSet->shmViewerSelf);
    shm_viewer_open(basic, &ipcSet->shmViewerSelf, ipcPath, sizeof(IpcBuffer));
    ipcSet->ipc = shm_viewer_memory_type(&ipcSet->shmViewerSelf, IpcBuffer);
    
    // Open the Master IPC buffer
    shm_viewer_init(&ipcSet->shmViewerMaster);
    shm_viewer_open(basic, &ipcSet->shmViewerMaster, masterIpcPath, sizeof(IpcBuffer));
    ipcSet->ipcMaster = shm_viewer_memory_type(&ipcSet->shmViewerMaster, IpcBuffer);
    
    // Open the LogWriter IPC buffer
    shm_viewer_init(&ipcSet->shmViewerLogWriter);
    shm_viewer_open(basic, &ipcSet->shmViewerLogWriter, logWriterIpcPath, sizeof(IpcBuffer));
    
    // Tell the log writer to open our log file
    ipc_set_log_file_open(basic, ipcSet, sourceId);
}

void ipc_set_deinit(R(IpcSet*) ipcSet)
{
    shm_viewer_close(&ipcSet->shmViewerSelf);
    shm_viewer_close(&ipcSet->shmViewerMaster);
    shm_viewer_close(&ipcSet->shmViewerLogWriter);
}

int ipc_set_receive(R(Basic*) basic, R(IpcSet*) ipcSet)
{
    R(IpcBuffer*) ipc = ipcSet->ipc;
    
    if (ipc_buffer_try(basic, ipc))
    {
        for (;;)
        {
            IpcPacket packet;
            
            if (!ipc_buffer_read(basic, ipc, &packet))
                break;
            
            if (ipc_packet_opcode(&packet) == ServerOp_Shutdown)
                return true;
            
            ipc_set_handle_packet(basic, &packet);
            
            ipc_packet_deinit(&packet);
        }
    }
    
    return false;
}

int ipc_set_send(R(Basic*) basic, R(IpcSet*) ipcSet, ServerOp opcode, int sourceId, uint32_t length, R(const void*) data)
{
    return ipc_buffer_write(basic, ipcSet->ipcMaster, opcode, sourceId, length, data);
}

void ipc_set_keep_alive(R(Basic*) basic, R(IpcSet*) ipcSet, int sourceId)
{
    uint64_t curTime = clock_milliseconds();
    
    if ((curTime - ipcSet->keepAliveTimestamp) >= EQP_IPC_KEEP_ALIVE_DELAY_MILLISECONDS)
    {
        ipc_set_send(basic, ipcSet, ServerOp_KeepAlive, sourceId, 0, NULL);
        ipcSet->keepAliveTimestamp = curTime;
    }
}

void ipc_set_log_file_control(R(Basic*) basic, R(IpcSet*) ipcSet, int sourceId, int open)
{
    ipc_buffer_write(basic, ipc_set_log_writer_ipc(ipcSet), open ? ServerOp_LogOpen : ServerOp_LogClose, sourceId, 0, NULL);
}
