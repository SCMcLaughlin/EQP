
#ifndef EQP_IPC_SET_H
#define EQP_IPC_SET_H

#include "define.h"
#include "server_op.h"
#include "ipc_buffer.h"
#include "share_mem.h"
#include "eqp_clock.h"

#define EQP_IPC_KEEP_ALIVE_DELAY_MILLISECONDS TIMER_SECONDS(5)

STRUCT_DECLARE(Basic);

STRUCT_DEFINE(IpcSet)
{
    IpcBuffer*      ipc;
    IpcBuffer*      ipcMaster;
    uint64_t        keepAliveTimestamp;
    ShmViewer       shmViewerSelf;
    ShmViewer       shmViewerMaster;
    ShmViewer       shmViewerLogWriter;
};

void    ipc_set_open(Basic* basic, IpcSet* ipcSet, int sourceId, const char* ipcPath, const char* masterIpcPath, const char* logWriterIpcPath);
void    ipc_set_deinit(IpcSet* ipcSet);

/*
    This is a special forward declaration -- no common implementation is provided, each binary 
    that links to non_master needs to provide its own implementation.
*/
void    ipc_set_handle_packet(Basic* basic, IpcPacket* packet);

// Returns true if ServerOp_Shutdown is received
int     ipc_set_receive(Basic* basic, IpcSet* ipcSet);
int     ipc_set_send(Basic* basic, IpcSet* ipcSet, ServerOp opcode, int sourceId, uint32_t length, const void* data);
void    ipc_set_keep_alive(Basic* basic, IpcSet* ipcSet, int sourceId);

void    ipc_set_log_file_control(Basic* basic, IpcSet* ipcSet, int sourceId, int open);
#define ipc_set_log_file_open(basic, set, sourceId) ipc_set_log_file_control((basic), (set), (sourceId), true)
#define ipc_set_log_file_close(basic, set, sourceId) ipc_set_log_file_control((basic), (set), (sourceId), false)

#define ipc_set_log_writer_ipc(set) shm_viewer_memory_type(&(set)->shmViewerLogWriter, IpcBuffer)

#endif//EQP_IPC_SET_H
