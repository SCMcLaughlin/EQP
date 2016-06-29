
#ifndef EQP_IPC_BUFFER_H
#define EQP_IPC_BUFFER_H

#include "define.h"
#include "eqp_clock.h"
#include "eqp_alloc.h"
#include "server_op.h"
#include "eqp_semaphore.h"
#include <stdatomic.h>

#define EQP_IPC_PACKET_MAX_SIZE 4096
#define EQP_IPC_BUFFER_SIZE MEGABYTES(2)
#define EQP_IPC_BUFFER_NUM_PACKETS (EQP_IPC_BUFFER_SIZE / EQP_IPC_PACKET_MAX_SIZE)

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(ShmViewer);
STRUCT_DECLARE(ShmCreator);

STRUCT_DEFINE(IpcPacket)
{
    ServerOp    opcode;
    int         sourceId;
    uint32_t    length;
    byte*       data;
};

STRUCT_DEFINE(IpcBlock)
{
    uint32_t    nextIndex;
    atomic_int  hasBeenRead;
    atomic_int  hasBeenWritten;
    ServerOp    opcode;
    int         sourceId;
    uint32_t    length;
    byte        data[EQP_IPC_PACKET_MAX_SIZE];
};

STRUCT_DEFINE(IpcBuffer)
{
    IpcBlock    blocks[EQP_IPC_BUFFER_NUM_PACKETS];
    atomic_uint readStart;
    atomic_uint readEnd;
    atomic_uint writeStart;
    atomic_uint writeEnd;
    Semaphore   semaphore;
};

void    ipc_buffer_init(Basic* basic, IpcBuffer* ipc);
void    ipc_buffer_shm_create_init(Basic* basic, IpcBuffer** ipc, ShmCreator* creator, ShmViewer* viewer, const char* path);

#define ipc_buffer_wait(basic, ipc) semaphore_wait((basic), &(ipc)->semaphore)
#define ipc_buffer_try(basic, ipc) semaphore_try_wait((basic), &(ipc)->semaphore)
int     ipc_buffer_write(Basic* basic, IpcBuffer* ipc, ServerOp opcode, int sourceId, uint32_t length, const void* data);
int     ipc_buffer_read(Basic* basic, IpcBuffer* ipc, IpcPacket* packet);

void    ipc_packet_init(Basic* basic, IpcPacket* packet, ServerOp opcode, int sourceId, uint32_t length, void* data);
void    ipc_packet_deinit(IpcPacket* packet);
#define ipc_packet_opcode(p) ((p)->opcode)
#define ipc_packet_source_id(p) ((p)->sourceId)
#define ipc_packet_length(p) ((p)->length)
#define ipc_packet_data(p) ((p)->data)
#define ipc_packet_data_type(p, type) (type*)ipc_packet_data(p)

#endif//EQP_IPC_BUFFER_H
