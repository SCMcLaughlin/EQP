
#include "zone_cluster_ipc.h"
#include "zone_cluster.h"

static void zc_ipc_handle(R(ZC*) zc, R(IpcPacket*) packet)
{
    ServerOp opcode     = ipc_packet_opcode(packet);
    int sourceId        = ipc_packet_source_id(packet);
    
    switch (opcode)
    {
    case ServerOp_StartZone:
        zc_start_zone(zc, sourceId);
        break;
    
    default:
        break;
    }
}

int zc_ipc_check(R(ZC*) zc)
{
    R(IpcBuffer*) ipc = zc->ipc;
    
    if (ipc_buffer_try(B(zc), ipc))
    {
        for (;;)
        {
            IpcPacket packet;
            
            if (!ipc_buffer_read(B(zc), ipc, &packet))
                break;
            
            if (ipc_packet_opcode(&packet) == ServerOp_Shutdown)
                return true;
            
            zc_ipc_handle(zc, &packet);
            
            ipc_packet_deinit(&packet);
        }
    }
    
    return false;
}
