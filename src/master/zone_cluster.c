
#include "zone_cluster.h"
#include "master_ipc.h"
#include "eqp_master.h"

ZoneCluster* zone_cluster_create(R(MasterIpcThread*) ipcThread, uint16_t id, uint16_t port, uint16_t maxZones)
{
    R(ZoneCluster*) zc = eqp_alloc_type(B(ipcThread), ZoneCluster);
    
    proc_init(&zc->proc);
    
    zc->portHostByteOrder   = port;
    zc->id                  = id;
    zc->maxZones            = maxZones;
    zc->reservedZones       = 0;
    zc->zoneCount           = 0;
    zc->ipcThread           = ipcThread;
    
    master_start_zone_cluster(master_ipc_thread_master(ipcThread), zc, id, port);
    
    return zc;
}

void zone_cluster_start_zone(R(ZoneCluster*) zc, int sourceId)
{
    R(Basic*) basic = B(zc->ipcThread);
    int zoneId      = sourceId % EQP_SOURCE_ID_ZONE_INSTANCE_OFFSET;
    int instId      = sourceId / EQP_SOURCE_ID_ZONE_INSTANCE_OFFSET;
    
    log_format(basic, LogInfo, "zone_cluster%u: starting sourceId %i (%s_instance%i)", zc->id, sourceId,
        zone_short_name_by_id(zoneId), instId);
    
    proc_ipc_send(basic, zone_cluster_proc(zc), ServerOp_StartZone, sourceId, 0, NULL);
}

void zone_cluster_set_max_zones(R(ZoneCluster*) zc, uint16_t maxZones)
{
    zc->maxZones = maxZones;
}
