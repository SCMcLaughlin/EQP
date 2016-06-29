
#include "packet_broadcast.h"
#include "client_packet_trilogy_output.h"

void packet_broadcast_init(ZC* zc, PacketBroadcast* packet, BroadcastEncoder encoder, const void* source)
{
    packet->source  = source;
    packet->encoder = encoder;
    packet->trilogy = NULL;
    
    packet->zc      = zc;
}

void packet_broadcast_set_packet_for_expansion(PacketBroadcast* packetBroadcast, int expansionId, void* packet)
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

PacketTrilogy* packet_broadcast_get_trilogy(PacketBroadcast* packet)
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

void packet_broadcast_encode_op_spawn(PacketBroadcast* packet, int expansionId)
{
    (void)packet;
    (void)expansionId;
}
