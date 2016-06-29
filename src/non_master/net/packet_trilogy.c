
#include "packet_trilogy.h"
#include "eqp_basic.h"

#define CRC_SIZE sizeof(uint32_t)

static uint32_t packet_trilogy_calc_length_and_frag_count(uint32_t length, uint16_t* fragCount)
{
    // Not trying to be as space-efficient as possible, bit headache-inducing...
    
    if (length <= EQP_PACKET_TRILOGY_DATA_SPACE)
    {
        *fragCount = 0;
        return sizeof(PacketTrilogy) + EQP_PACKET_TRILOGY_HEADER_SIZE + length + CRC_SIZE;
    }
    else
    {
        uint32_t count  = length / EQP_PACKET_TRILOGY_DATA_SPACE;
        uint32_t rem    = length % EQP_PACKET_TRILOGY_DATA_SPACE;
        
        if (rem)
            count++;
        
        *fragCount = count - 1;
        return sizeof(PacketTrilogy) + (EQP_PACKET_TRILOGY_HEADER_SIZE * count) + length + (CRC_SIZE * count);
    }
}

PacketTrilogy* packet_trilogy_create(Basic* basic, uint16_t opcode, uint32_t length)
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

void packet_trilogy_grab(PacketTrilogy* packet)
{
    atomic_fetch_add(&packet->refCount, 1);
}

void packet_trilogy_drop(PacketTrilogy* packet)
{
    int refCount = atomic_fetch_sub(&packet->refCount, 1);
    
    if (refCount <= 1)
        free(packet);
}

void packet_trilogy_fragmentize(PacketTrilogy* packet)
{
    uint16_t n = packet->fragCount;
    uint32_t dataLength;
    uint32_t dst;
    uint32_t src;
    uint16_t i;
    uint32_t j;
    byte* data;
    
    if (n == 0)
        return;
    
    dataLength  = packet->dataLength;
    src         = dataLength + EQP_PACKET_TRILOGY_DATA_OFFSET - 1;
    dst         = packet_trilogy_length_raw(packet) - CRC_SIZE - 1;
    data        = packet->data;
    
    // The opcode counts towards the size of the first fragment; any non-final fragment
    // that doesn't make use of its full size will be rejected by the client. We add
    // the size of the opcode here so that 2 extra bytes will go towards the final
    // packet, effectively shifting everything over without increasing the 'actual'
    // data size. (Yes, it is a bit confusing.)
    dataLength += sizeof(uint16_t);
    dataLength %= EQP_PACKET_TRILOGY_DATA_SPACE;
    
    if (dataLength == 0)
        dataLength = EQP_PACKET_TRILOGY_DATA_SPACE;
    
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < dataLength; j++)
        {
            data[dst--] = data[src--];
        }
        
        dst -= EQP_PACKET_TRILOGY_DATA_OFFSET + CRC_SIZE;
        dataLength = EQP_PACKET_TRILOGY_DATA_SPACE;
    }
}

#undef CRC_SIZE
