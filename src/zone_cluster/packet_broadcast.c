
#include "packet_broadcast.h"
#include "client_packet_trilogy_output.h"

void packet_broadcast_init(R(ZC*) zc, R(PacketBroadcast*) packet, BroadcastEncoder encoder, R(const void*) source)
{
    packet->source  = source;
    packet->encoder = encoder;
    packet->trilogy = NULL;
    
    packet->zc      = zc;
}

void packet_broadcast_set_packet_for_expansion(R(PacketBroadcast*) packetBroadcast, int expansionId, R(void*) packet)
{
    switch (expansionId)
    {
    case ExpansionId_Trilogy:
        packetBroadcast->trilogy = (PacketTrilogy*)packet;
        packet_trilogy_fragmentize(packet);
        break;
    
    default:
        break;
    }
}

PacketTrilogy* packet_broadcast_get_trilogy(R(PacketBroadcast*) packet)
{
    if (packet->trilogy)
        goto ret;
    
    packet->encoder(packet, ExpansionId_Trilogy);
    
    if (packet->trilogy)
        packet_trilogy_fragmentize(packet->trilogy);
    
ret:
    return packet->trilogy;
}

/* Encoding dispatchers */

void packet_broadcast_encode_op_spawn(R(PacketBroadcast*) packet, int expansionId)
{
    (void)packet;
    (void)expansionId;
}
