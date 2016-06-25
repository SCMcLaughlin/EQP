
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
    
    master_start_zone_cluster(master_ipc_thread_master(ipcThread), zc, id, port);
    
    return zc;
}

void zone_cluster_set_max_zones(R(ZoneCluster*) zc, uint16_t maxZones)
{
    zc->maxZones = maxZones;
}
