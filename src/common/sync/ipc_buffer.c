
#include "ipc_buffer.h"
#include "eqp_basic.h"
#include "share_mem.h"

void ipc_buffer_init(R(Basic*) basic, R(IpcBuffer*) ipc)
{
    uint32_t i;
    uint32_t n          = EQP_IPC_BUFFER_NUM_PACKETS - 1;
    IpcBlock* blocks    = ipc->blocks;
    
    blocks[0].nextIndex = 1;
    atomic_init(&blocks[0].hasBeenRead, 0);
    atomic_init(&blocks[0].hasBeenWritten, 0);
    
    for (i = 1; i < n; i++)
    {
        blocks[i].nextIndex = i + 1;
        atomic_init(&blocks[i].hasBeenRead, 0);
        atomic_init(&blocks[i].hasBeenWritten, 0);
    }
    
    blocks[n].nextIndex = 0;
    atomic_init(&blocks[n].hasBeenRead, 0);
    atomic_init(&blocks[n].hasBeenWritten, 0);
    
    atomic_init(&ipc->readStart, 0);
    atomic_init(&ipc->readEnd, 0);
    atomic_init(&ipc->writeStart, 0);
    atomic_init(&ipc->writeEnd, 0);
    
    semaphore_init(basic, &ipc->semaphore);
}

void ipc_buffer_shm_create_init(R(Basic*) basic, R(IpcBuffer**) ipc, R(ShmCreator*) creator, R(ShmViewer*) viewer, R(const char*) path)
{
    share_mem_create(basic, creator, viewer, path, sizeof(IpcBuffer));
    *ipc = shm_viewer_memory_type(viewer, IpcBuffer);
    ipc_buffer_init(basic, *ipc);
}

static void ipc_buffer_do_write(R(Basic*) basic, R(IpcBuffer*) ipc, R(IpcBlock*) block, ServerOp opcode, int sourceId, uint32_t length, R(const void*) data)
{
    int written = 1;
    
    if (length > EQP_IPC_PACKET_MAX_SIZE)
        length = EQP_IPC_PACKET_MAX_SIZE;
    
    block->opcode   = opcode;
    block->sourceId = sourceId;
    block->length   = length;
    
    if (length && data)
        memcpy(block->data, data, length);
    
    atomic_store(&block->hasBeenWritten, 1);
    
    // Advance over completely written blocks, as far as possible
    for (;;)
    {
        uint32_t index  = atomic_load(&ipc->writeStart);
        block           = &ipc->blocks[index];
        
        // If the hasBeenWritten flag has already been cleared, another thread must have already advanced for us
        // Or, we've reached the end of what was available
        if (!atomic_compare_exchange_strong(&block->hasBeenWritten, &written, 0))
            break;
        
        // Advance writeStart and loop
        atomic_compare_exchange_strong(&ipc->writeStart, &index, block->nextIndex);
    }
    
    semaphore_trigger(basic, &ipc->semaphore);
}

int ipc_buffer_write(R(Basic*) basic, R(IpcBuffer*) ipc, ServerOp opcode, int sourceId, uint32_t length, R(const void*) data)
{
    uint64_t timestamp = clock_milliseconds();
    
    for (;;)
    {
        uint32_t index      = atomic_load(&ipc->writeEnd);
        R(IpcBlock*) block  = &ipc->blocks[index];
        
        if (block->nextIndex == atomic_load(&ipc->readStart))
        {
            if ((clock_milliseconds() - timestamp) >= 10)
                return false;
            
            continue;
        }
        
        if (atomic_compare_exchange_weak(&ipc->writeEnd, &index, block->nextIndex))
        {
            ipc_buffer_do_write(basic, ipc, block, opcode, sourceId, length, data);
            return true;
        }
    }
}

static void ipc_buffer_do_read(R(Basic*) basic, R(IpcBuffer*) ipc, R(IpcPacket*) packet, R(IpcBlock*) block)
{
    int read = 1;
    
    ipc_packet_init(basic, packet, block->opcode, block->sourceId, block->length, block->data);
    
    atomic_store(&block->hasBeenRead, 1);
    
    // Advance over completely read blocks, as far as possible
    for (;;)
    {
        uint32_t index  = atomic_load(&ipc->readStart);
        block           = &ipc->blocks[index];
        
        // If the hasBeenRead flag has already been cleared, another thread must have already advanced for us
        // Or, we've reached the end of what was available
        if (!atomic_compare_exchange_strong(&block->hasBeenRead, &read, 0))
            return;
        
        // Advance readStart and loop
        atomic_compare_exchange_strong(&ipc->readStart, &index, block->nextIndex);
    }
}

int ipc_buffer_read(R(Basic*) basic, R(IpcBuffer*) ipc, R(IpcPacket*) packet)
{
    for (;;)
    {
        uint32_t index      = atomic_load(&ipc->readEnd);
        R(IpcBlock*) block  = &ipc->blocks[index];
        
        if (index == atomic_load(&ipc->writeStart))
            return false;
        
        if (atomic_compare_exchange_weak(&ipc->readEnd, &index, block->nextIndex))
        {
            ipc_buffer_do_read(basic, ipc, packet, block);
            return true;
        }
    }
}

void ipc_packet_init(R(Basic*) basic, R(IpcPacket*) packet, ServerOp opcode, int sourceId, uint32_t length, R(void*) vdata)
{
    packet->opcode      = opcode;
    packet->sourceId    = sourceId;
    packet->length      = length;
    
    if (length && vdata)
    {
        byte* data = eqp_alloc_type_bytes(basic, length, byte);
        memcpy(data, vdata, length);
        packet->data = data;
    }
    else
    {
        packet->data = NULL;
    }
}

void ipc_packet_deinit(R(IpcPacket*) packet)
{
    if (packet->data)
    {
        free(packet->data);
        packet->data = NULL;
    }
}
