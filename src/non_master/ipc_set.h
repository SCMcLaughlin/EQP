
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

void    ipc_set_open(R(Basic*) basic, R(IpcSet*) ipcSet, int sourceId, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath);
void    ipc_set_deinit(R(IpcSet*) ipcSet);

/*
    This is a special forward declaration -- no common implementation is provided, each binary 
    that links to non_master needs to provide its own implementation.
*/
void    ipc_set_handle_packet(R(Basic*) basic, R(IpcPacket*) packet);

// Returns true if ServerOp_Shutdown is received
int     ipc_set_receive(R(Basic*) basic, R(IpcSet*) ipcSet);
int     ipc_set_send(R(Basic*) basic, R(IpcSet*) ipcSet, ServerOp opcode, int sourceId, uint32_t length, R(const void*) data);
void    ipc_set_keep_alive(R(Basic*) basic, R(IpcSet*) ipcSet, int sourceId);

void    ipc_set_log_file_control(R(Basic*) basic, R(IpcSet*) ipcSet, int sourceId, int open);
#define ipc_set_log_file_open(basic, set, sourceId) ipc_set_log_file_control((basic), (set), (sourceId), true)
#define ipc_set_log_file_close(basic, set, sourceId) ipc_set_log_file_control((basic), (set), (sourceId), false)

#define ipc_set_log_writer_ipc(set) shm_viewer_memory_type(&(set)->shmViewerLogWriter, IpcBuffer)

#endif//EQP_IPC_SET_H
