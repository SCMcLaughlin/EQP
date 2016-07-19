
#include "zone_cluster_ipc.h"
#include "zone_cluster.h"

void ipc_set_handle_packet(Basic* basic, IpcPacket* packet)
{
    ZC* zc          = (ZC*)basic;
    ServerOp opcode = ipc_packet_opcode(packet);
    int sourceId    = ipc_packet_source_id(packet);
    
    switch (opcode)
    {
    case ServerOp_StartZone:
        zc_start_zone(zc, sourceId);
        break;
    
    case ServerOp_ClientZoning:
        zc_client_expected_to_zone_in(zc, sourceId, packet);
        break;
    
    case ServerOp_ItemSharedMemoryOpen:
        zc_items_open(zc, packet);
        break;
    
    default:
        break;
    }
}
