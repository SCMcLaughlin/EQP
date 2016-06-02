
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

void    ipc_buffer_init(R(Basic*) basic, R(IpcBuffer*) ipc);

#define ipc_buffer_wait(basic, ipc) semaphore_wait((basic), &(ipc)->semaphore)
int     ipc_buffer_write(R(Basic*) basic, R(IpcBuffer*) ipc, ServerOp opcode, int sourceId, uint32_t length, R(void*) data);
int     ipc_buffer_read(R(Basic*) basic, R(IpcBuffer*) ipc, R(IpcPacket*) packet);

void    ipc_packet_init(R(Basic*) basic, R(IpcPacket*) packet, ServerOp opcode, int sourceId, uint32_t length, R(void*) data);
void    ipc_packet_deinit(R(IpcPacket*) packet);
#define ipc_packet_opcode(p) ((p)->opcode)
#define ipc_packet_source_id(p) ((p)->sourceId)
#define ipc_packet_length(p) ((p)->length)
#define ipc_packet_data(p) ((p)->data)

#endif//EQP_IPC_BUFFER_H
