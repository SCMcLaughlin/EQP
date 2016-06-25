
#ifndef EQP_ZONE_CLUSTER_MANAGER_H
#define EQP_ZONE_CLUSTER_MANAGER_H

#include "define.h"
#include "zone_cluster.h"
#include "eqp_array.h"
#include "lua_sys.h"
#include "zone_id.h"
#include "child_process.h"

#define EQP_ZONE_CLUSTER_DEFAULT_ZONES_PER_PROCESS  8
#define EQP_ZONE_CLUSTER_PORT_RANGE_START           7000

STRUCT_DECLARE(MasterIpcThread);

STRUCT_DEFINE(ZoneClusterMgr)
{
    uint16_t            maxZonesPerCluster;
    uint16_t            nextZoneClusterPort;
    Array*              activeZoneClusters;
    Array*              zoneClustersBySourceId;
    Array*              zoneReservations;
    ZoneShortNameMap*   zoneShortNameMap;
    MasterIpcThread*    ipcThread;
};

STRUCT_DEFINE(ZoneReservation)
{
    int             sourceId;
    int             alwaysUp;
    ZoneCluster*    zoneCluster;
};

void            zc_mgr_init(R(MasterIpcThread*) ipcThread, R(ZoneClusterMgr*) mgr, R(lua_State*) L);
void            zc_mgr_deinit(R(ZoneClusterMgr*) mgr);

ZoneCluster*    zc_mgr_get(R(ZoneClusterMgr*) mgr, int sourceId);

EQP_API void            zc_mgr_set_max_zones_per_cluster(R(ZoneClusterMgr*) mgr, uint16_t maxPer);
EQP_API ZoneCluster*    zc_mgr_start_zone_cluster(R(ZoneClusterMgr*) mgr);
EQP_API void            zc_mgr_add_zone_reservation(R(ZoneClusterMgr*) mgr, R(ZoneCluster*) zc, R(const char*) shortName, int zoneId, int instanceId, int alwaysUp);

#endif//EQP_ZONE_CLUSTER_MANAGER_H
