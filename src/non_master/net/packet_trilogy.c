
#include "packet_trilogy.h"
#include "eqp_basic.h"

#define MAX_HEADER_SIZE (sizeof(uint16_t) * 9)
#define CRC_SIZE sizeof(uint32_t)
#define DATA_OFFSET MAX_HEADER_SIZE
#define DATA_SPACE (512 - MAX_HEADER_SIZE - CRC_SIZE)

static uint32_t packet_trilogy_calc_length_and_frag_count(uint32_t length, uint16_t* fragCount)
{
    // Not trying to be as space-efficient as possible, bit headache-inducing...
    
    if (length <= DATA_SPACE)
    {
        *fragCount = 0;
        return sizeof(PacketTrilogy) + MAX_HEADER_SIZE + length + CRC_SIZE;
    }
    else
    {
        uint32_t count  = length / DATA_SPACE;
        uint32_t rem    = length % DATA_SPACE;
        
        if (rem)
            count++;
        
        *fragCount = count - 1;
        return sizeof(PacketTrilogy) + (MAX_HEADER_SIZE * count) + length + (CRC_SIZE * count);
    }
}

PacketTrilogy* packet_trilogy_create(R(Basic*) basic, uint16_t opcode, uint32_t length)
{
    PacketTrilogy* packet;
    uint16_t fragCount;
    uint32_t allocLength = packet_trilogy_calc_length_and_frag_count(length, &fragCount);
    
    packet = eqp_alloc_type_bytes(basic, allocLength, PacketTrilogy);
    
    packet->opcode      = opcode;
    packet->fragCount   = fragCount;
    packet->dataLength  = length;
    atomic_init(&packet->refCount, 0);
    
    return packet;
}

void packet_trilogy_grab(R(PacketTrilogy*) packet)
{
    atomic_fetch_add(&packet->refCount, 1);
}

void packet_trilogy_drop(R(PacketTrilogy*) packet)
{
    int refCount = atomic_fetch_sub(&packet->refCount, 1);
    
    if (refCount <= 1)
        free(packet);
}

void packet_trilogy_fragmentize(R(PacketTrilogy*) packet)
{
    (void)packet;
}

#undef MAX_HEADER_SIZE
#undef CRC_SIZE
#undef DATA_OFFSET
#undef DATA_SPACE
