
#ifndef EQP_PACKET_TRILOGY_H
#define EQP_PACKET_TRILOGY_H

#include "define.h"
#include "netcode.h"
#include "eqp_alloc.h"
#include "crc.h"
#include "aligned.h"
#include <stdatomic.h>

/*
    The full packet header looks something like this. We work backwards from the opcode;
    all fields are filled in at the protocol level, just before sending-time

    struct CompleteHeader
    {
        uint16_t    header;
        uint16_t    sequence;
        uint16_t    ackResponse;
        uint16_t    ackRequest;
        uint16_t    fragIndex;
        uint16_t    fragGroup;
        uint16_t    fragsInGroup;
        uint8_t     counterHigh;
        uint8_t     counterLow;
        uint16_t    opcode;
    };
*/

STRUCT_DECLARE(Basic);

ENUM_DEFINE(PacketTrilogyHeaderBits)
{
    PacketTrilogyUnknownBit0            = 0x0001,
    PacketTrilogyHasAckRequest          = 0x0002,
    PacketTrilogyIsClosing              = 0x0004,
    PacketTrilogyIsFragment             = 0x0008,
    PacketTrilogyHasAckCounter          = 0x0010,
    PacketTrilogyIsFirstPacket          = 0x0020,
    PacketTrilogyIsClosing2             = 0x0040,
    PacketTrilogyIsSequenceEnd          = 0x0080,
    PacketTrilogyIsSpecialAckRequest    = 0x0100,
    PacketTrilogyUnknownBit9            = 0x0200,
    PacketTrilogyHasAckResponse         = 0x0400,
    PacketTrilogyUnknownBit11           = 0x0800,
    PacketTrilogyUnknownBit12           = 0x1000,
    PacketTrilogyUnknownBit13           = 0x2000,
    PacketTrilogyUnknownBit14           = 0x4000,
    PacketTrilogyUnknownBit15           = 0x8000
};

STRUCT_DEFINE(PacketTrilogy)
{
    uint16_t    opcode;
    uint16_t    fragCount;
    uint32_t    dataLength;
    atomic_int  refCount;
    byte        data[0];
};

PacketTrilogy*  packet_trilogy_create(R(Basic*) basic, uint16_t opcode, uint32_t length);
void            packet_trilogy_grab(R(PacketTrilogy*) packet);
void            packet_trilogy_drop(R(PacketTrilogy*) packet);

#define         packet_trilogy_data(packet) ((packet)->data)
#define         packet_trilogy_length(packet) ((packet)->dataLength)

void            packet_trilogy_fragmentize(R(PacketTrilogy*) packet);

#endif//EQP_PACKET_TRILOGY_H
