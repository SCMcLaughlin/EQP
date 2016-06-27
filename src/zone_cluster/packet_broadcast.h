
#ifndef EQP_PACKET_BROADCAST_H
#define EQP_PACKET_BROADCAST_H

#include "define.h"
#include "packet_trilogy.h"
#include "expansion.h"

STRUCT_DECLARE(PacketBroadcast);
STRUCT_DECLARE(ZC);

typedef void (*BroadcastEncoder)(R(PacketBroadcast*) packet, int expansionId);

STRUCT_DEFINE(PacketBroadcast)
{
    const void*         source;
    BroadcastEncoder    encoder;
    PacketTrilogy*      trilogy;
    //Packet*             byExpansion[?]; //need to handle one -> many packets, too
    ZC*                 zc;
};

void            packet_broadcast_init(R(ZC*) zc, R(PacketBroadcast*) packet, BroadcastEncoder encoder, R(const void*) source);

void            packet_broadcast_set_packet_for_expansion(R(PacketBroadcast*) packetBroadcast, int expansionId, R(void*) packet);
PacketTrilogy*  packet_broadcast_get_trilogy(R(PacketBroadcast*) packet);

/* Encoding dispatchers */
void    packet_broadcast_encode_op_spawn(R(PacketBroadcast*) packet, int expansionId);

#endif//EQP_PACKET_BROADCAST_H
