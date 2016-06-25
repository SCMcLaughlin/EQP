
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
    
    /*if (lua_sys_run_file_no_throw(B(ipcThread), L, ZONE_CLUSTER_CONFIG, 1))
    {
        int n;
        int i;
        
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            log_format(B(ipcThread), LogLua, "[zc_mgr_init] Expected config script '" ZONE_CLUSTER_CONFIG "' to return a table; aborting zone cluster initialization");
            return;
        }
        
        // Get the global maxZonesPerCluster setting, if one was given
        lua_getfield(L, -1, "maxzonespercluster");
        if (lua_isnumber(L, -1))
            mgr->maxZonesPerCluster = lua_tointeger(L, -1);
        lua_pop(L, 1);
        
        n = lua_objlen(L, -1);
        
        for (i = 1; i <= n; i++)
        {
            R(ZoneCluster*) zc;
            int m;
            int j;
            
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            
            if (!lua_istable(L, -1))
                goto skip_cluster;
            
            // A Cluster entry is now on the stack
            zc = zc_mgr_start_zone_cluster(mgr);
            
            // Is there a maxZones setting for this specific cluster?
            lua_getfield(L, -1, "maxzones");
            if (lua_isnumber(L, -1))
                zone_cluster_set_max_zones(zc, lua_tointeger(L, -1));
            lua_pop(L, 1);
            
            m = lua_objlen(L, -1);
            
            for (j = 1; j <= m; j++)
            {
                ZoneReservation res;
                R(const char*) shortName;
                
                res.sourceId    = 0;
                res.alwaysUp    = 0;
                res.zoneCluster = zc;
                
                lua_pushinteger(L, j);
                lua_gettable(L, -2);
                
                if (!lua_istable(L, -1))
                    goto skip_zone;
                
                // A Zone entry is now on the stack
                lua_getfield(L, -1, "shortname");
                if (lua_isstring(L, -1))
                    res.sourceId = zone_id_by_short_name(mgr->zoneShortNameMap, lua_tostring(L, -1), lua_objlen(L, -1));
                lua_pop(L, 1);
                
                lua_getfield(L, -1, "zoneid");
                if (lua_isnumber(L, -1))
                    res.sourceId = lua_tointeger(L, -1);
                
                if (res.sourceId <= 0 || res.sourceId > EQP_SOURCE_ID_ZONE_MAX)
                    goto skip_zone;
                
                //lua_getfield(L, -1, "instanceid");
                //if (lua_isnumber(L, -1))
                
                //create reservation
                //if alwayson is set, send the signal to start the zone
                
            skip_zone:
                lua_pop(L, 1);
            }
            
        skip_cluster:
            lua_pop(L, 1); // pop Cluster entry
        }
        
        lua_pop(L, 1);
    }
    */
}

void zc_mgr_deinit(R(ZoneClusterMgr*) mgr)
{
    if (mgr->activeZoneClusters)
    {
        R(ZoneCluster**) array  = array_data_type(mgr->activeZoneClusters, ZoneCluster*);
        uint32_t n              = array_count(mgr->activeZoneClusters);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            zone_cluster_destroy(array[i]);
        }
        
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

void zc_mgr_set_max_zones_per_cluster(R(ZoneClusterMgr*) mgr, uint16_t maxPer)
{
    mgr->maxZonesPerCluster = maxPer;
}

ZoneCluster* zc_mgr_start_zone_cluster(R(ZoneClusterMgr*) mgr)
{
    R(MasterIpcThread*) ipcThread   = mgr->ipcThread;
    uint16_t id                     = array_count(mgr->activeZoneClusters);
    ZoneClusterBySourceId bySrc;
    R(ZoneCluster*) zc;
    
    zc = zone_cluster_create(ipcThread, id, mgr->nextZoneClusterPort++, mgr->maxZonesPerCluster);
    
    bySrc.sourceId      = ((int)id) + EQP_SOURCE_ID_ZONE_CLUSTER_OFFSET;
    bySrc.zoneCluster   = zc;
    
    array_push_back(B(ipcThread), &mgr->activeZoneClusters, (void*)&zc);
    array_push_back(B(ipcThread), &mgr->zoneClustersBySourceId, &bySrc);
    
    return zc;
}

void zc_mgr_add_zone_reservation(R(ZoneClusterMgr*) mgr, R(ZoneCluster*) zc, R(const char*) shortName, int zoneId, int instanceId, int alwaysUp)
{
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
    
    array_push_back(B(mgr->ipcThread), &mgr->zoneReservations, &res);
    
    zone_cluster_increment_reserved_zones(zc);
    
    if (alwaysUp)
    {
        //fixme: start the zone
        zone_cluster_increment_zone_count(zc);
    }
}

#undef ZONE_CLUSTER_CONFIG
