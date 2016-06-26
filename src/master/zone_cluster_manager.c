
#include "zone_cluster_manager.h"
#include "master_ipc.h"

#define ZONE_CLUSTER_CONFIG "scripts/master/zone_cluster_config_loader.lua"

void zc_mgr_init(R(MasterIpcThread*) ipcThread, R(ZoneClusterMgr*) mgr, R(lua_State*) L)
{
    mgr->maxZonesPerCluster     = EQP_ZONE_CLUSTER_DEFAULT_ZONES_PER_PROCESS;
    mgr->nextZoneClusterPort    = EQP_ZONE_CLUSTER_PORT_RANGE_START;
    mgr->activeZoneClusters     = array_create_type(B(ipcThread), ZoneCluster*);
    mgr->zoneClustersBySourceId = array_create_type(B(ipcThread), ZoneClusterBySourceId);
    mgr->zoneReservations       = array_create_type(B(ipcThread), ZoneReservation);
    mgr->zoneShortNameMap       = zone_short_name_map_create(B(ipcThread));
    mgr->ipcThread              = ipcThread;
    
    if (lua_sys_run_file_no_throw(B(ipcThread), L, ZONE_CLUSTER_CONFIG, 1))
    {
        if (!lua_isfunction(L, -1))
        {
            log_format(B(ipcThread), LogLua, "[zc_mgr_init] Expected config script '" ZONE_CLUSTER_CONFIG "' to return a function; aborting zone cluster initialization");
            lua_pop(L, 1);
            return;
        }
        
        lua_pushlightuserdata(L, mgr);
        lua_sys_call_no_throw(B(ipcThread), L, 1, 0);
    }
}

void zc_mgr_deinit(R(ZoneClusterMgr*) mgr)
{
    if (mgr->activeZoneClusters)
    {
        // Master is responsible for destroying the actual individual zone clusters
        // otherwise we run into an issue where it's trying to send shutdown signals after
        // the IPC path has already been closed.
        array_destroy(mgr->activeZoneClusters);
        mgr->activeZoneClusters = NULL;
    }
    
    if (mgr->zoneClustersBySourceId)
    {
        array_destroy(mgr->zoneClustersBySourceId);
        mgr->zoneClustersBySourceId = NULL;
    }
    
    if (mgr->zoneReservations)
    {
        array_destroy(mgr->zoneReservations);
        mgr->zoneReservations = NULL;
    }
    
    if (mgr->zoneShortNameMap)
    {
        zone_short_name_map_destroy(mgr->zoneShortNameMap);
        mgr->zoneShortNameMap = NULL;
    }
}

static ZoneCluster* zc_mgr_start_zone_from_reserved_list(R(ZoneClusterMgr*) mgr, int zoneSourceId)
{
    R(ZoneReservation*) array   = array_data_type(mgr->zoneReservations, ZoneReservation);
    uint32_t n                  = array_count(mgr->zoneReservations);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        R(ZoneReservation*) res = &array[i];
        
        if (res->sourceId == zoneSourceId)
        {
            R(ZoneCluster*) zc = res->zoneCluster;
            
            zc_mgr_start_zone_on_cluster(mgr, zc, zoneSourceId);
            return zc;
        }
    }
    
    return NULL;
}

static ZoneCluster* zc_mgr_start_zone_from_free_space(R(ZoneClusterMgr*) mgr, int zoneSourceId)
{
    R(ZoneCluster**) array  = array_data_type(mgr->activeZoneClusters, ZoneCluster*);
    uint32_t n              = array_count(mgr->activeZoneClusters);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        R(ZoneCluster*) zc = array[i];
        
        if (zone_cluster_has_free_space(zc))
        {
            zc_mgr_start_zone_on_cluster(mgr, zc, zoneSourceId);
            return zc;
        }
    }
    
    return NULL;
}

ZoneCluster* zc_mgr_get_or_start(R(ZoneClusterMgr*) mgr, int zoneSourceId)
{
    R(ZoneCluster*) zc;
    
    // Is the zone already running?
    zc = zc_mgr_get(mgr, zoneSourceId);
    if (zc)
        goto ret;
    
    // Is the zone reserved by an active ZoneCluster, but not yet started?
    zc = zc_mgr_start_zone_from_reserved_list(mgr, zoneSourceId);
    if (zc)
        goto ret;
    
    // Is there an active ZoneCluster with space for another zone?
    zc = zc_mgr_start_zone_from_free_space(mgr, zoneSourceId);
    if (zc)
        goto ret;
    
    // If all else fails, start a brand new ZoneCluster process
    zc = zc_mgr_start_zone_cluster(mgr);
    zc_mgr_start_zone_on_cluster(mgr, zc, zoneSourceId);
    
ret:
    return zc;
}

ZoneCluster* zc_mgr_get(R(ZoneClusterMgr*) mgr, int sourceId)
{
    R(ZoneClusterBySourceId*) array = array_data_type(mgr->zoneClustersBySourceId, ZoneClusterBySourceId);
    uint32_t n                      = array_count(mgr->zoneClustersBySourceId);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        if (array[i].sourceId == sourceId)
            return array[i].zoneCluster;
    }
    
    return NULL;
}

void zc_mgr_start_zone_on_cluster(R(ZoneClusterMgr*) mgr, R(ZoneCluster*) zc, int sourceId)
{
    ZoneClusterBySourceId bySrc;
    
    zone_cluster_increment_zone_count(zc);
    zone_cluster_start_zone(zc, sourceId);
    
    bySrc.sourceId      = sourceId;
    bySrc.zoneCluster   = zc;
    
    array_push_back(B(mgr->ipcThread), &mgr->zoneClustersBySourceId, &bySrc);
}

void zc_mgr_set_max_zones_per_cluster(R(ZoneClusterMgr*) mgr, uint16_t maxPer)
{
    mgr->maxZonesPerCluster = maxPer;
    log_format(B(mgr->ipcThread), LogInfo, "Adjusted max zones per zone cluster to %u", maxPer);
}

ZoneCluster* zc_mgr_start_zone_cluster(R(ZoneClusterMgr*) mgr)
{
    R(MasterIpcThread*) ipcThread   = mgr->ipcThread;
    uint16_t id                     = array_count(mgr->activeZoneClusters);
    uint16_t port                   = mgr->nextZoneClusterPort++;
    ZoneClusterBySourceId bySrc;
    R(ZoneCluster*) zc;
    
    zc = zone_cluster_create(ipcThread, id, port, mgr->maxZonesPerCluster);
    
    log_format(B(ipcThread), LogInfo, "Started zone_cluster%u on port %u", id, port);
    
    bySrc.sourceId      = ((int)id) + EQP_SOURCE_ID_ZONE_CLUSTER_OFFSET;
    bySrc.zoneCluster   = zc;
    
    array_push_back(B(ipcThread), &mgr->activeZoneClusters, (void*)&zc);
    array_push_back(B(ipcThread), &mgr->zoneClustersBySourceId, &bySrc);
    
    return zc;
}

void zc_mgr_add_zone_reservation(R(ZoneClusterMgr*) mgr, R(ZoneCluster*) zc, R(const char*) shortName, int zoneId, int instanceId, int alwaysUp)
{
    R(Basic*) basic = B(mgr->ipcThread);
    ZoneReservation res;
    
    res.sourceId = 0;
    
    if (zoneId)
        res.sourceId = zoneId;
    else if (shortName)
        res.sourceId = zone_id_by_short_name(mgr->zoneShortNameMap, shortName, strlen(shortName));
    
    if (res.sourceId <= 0 || res.sourceId > EQP_SOURCE_ID_ZONE_MAX || instanceId < 0 || instanceId > EQP_SOURCE_ID_ZONE_INSTANCE_MAX)
        return;
    
    res.sourceId += instanceId * EQP_SOURCE_ID_ZONE_INSTANCE_OFFSET;

    res.alwaysUp    = alwaysUp;
    res.zoneCluster = zc;
    
    array_push_back(basic, &mgr->zoneReservations, &res);
    
    log_format(basic, LogInfo, "zone_cluster%u: reserved sourceId %i (%s_instance%i)", zone_cluster_id(zc), res.sourceId,
        shortName ? shortName : zone_short_name_by_id(zoneId), instanceId);
    
    zone_cluster_increment_reserved_zones(zc);
    
    if (alwaysUp)
        zc_mgr_start_zone_on_cluster(mgr, zc, res.sourceId);
}

#undef ZONE_CLUSTER_CONFIG
