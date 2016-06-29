
#ifndef EQP_PACKET_BROADCAST_H
#define EQP_PACKET_BROADCAST_H

#include "define.h"
#include "packet_trilogy.h"
#include "expansion.h"

STRUCT_DECLARE(PacketBroadcast);
STRUCT_DECLARE(ZC);

typedef void (*BroadcastEncoder)(PacketBroadcast* packet, int expansionId);

STRUCT_DEFINE(PacketBroadcast)
{
    const void*         source;
    BroadcastEncoder    encoder;
    PacketTrilogy*      trilogy;
    //Packet*             byExpansion[?]; //need to handle one -> many packets, too
    ZC*                 zc;
};

void            packet_broadcast_init(ZC* zc, PacketBroadcast* packet, BroadcastEncoder encoder, const void* source);

void            packet_broadcast_set_packet_for_expansion(PacketBroadcast* packetBroadcast, int expansionId, void* packet);
PacketTrilogy*  packet_broadcast_get_trilogy(PacketBroadcast* packet);

/* Encoding dispatchers */
void    packet_broadcast_encode_op_spawn(PacketBroadcast* packet, int expansionId);

#endif//EQP_PACKET_BROADCAST_H
