
#include "zone_cluster.h"

void zc_init(R(ZC*) zc, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath, R(const char*) sourceId, R(const char*) port)
{
    (void)ipcPath;
    (void)masterIpcPath;
    
    zc->sourceId = strtol(sourceId, NULL, 10) + EQP_SOURCE_ID_ZONE_CLUSTER_OFFSET;
    
    timer_pool_init(B(zc), &zc->timerPool);
    
    // IPC
    shm_viewer_init(&zc->shmViewerLogWriter);
    shm_viewer_open(B(zc), &zc->shmViewerLogWriter, logWriterIpcPath, sizeof(IpcBuffer));
    // Tell the log writer to open our log file
    ipc_buffer_write(B(zc), shm_viewer_memory_type(&zc->shmViewerLogWriter, IpcBuffer), ServerOp_LogOpen, zc->sourceId, 0, NULL);
    
    core_init(C(zc), zc->sourceId, shm_viewer_memory_type(&zc->shmViewerLogWriter, IpcBuffer));
    
    // Arrays
    zc->zoneList = array_create_type(B(zc), ZoneBySourceId);
    
    // Lua
    zc_lua_init(zc);
    
    // UDP socket
    zc->socket = udp_socket_create(B(zc));
    udp_socket_open(zc->socket, strtol(port, NULL, 10));
}

void zc_deinit(R(ZC*) zc)
{
    core_deinit(C(zc));
    shm_viewer_close(&zc->shmViewerLogWriter);
    
    if (zc->L)
    {
        lua_close(zc->L);
        zc->L = NULL;
    }
    
    if (zc->socket)
    {
        udp_socket_destroy(zc->socket);
        zc->socket = NULL;
    }
    
    if (zc->zoneList)
    {
        R(ZoneBySourceId*) array    = array_data_type(zc->zoneList, ZoneBySourceId);
        uint32_t n                  = array_count(zc->zoneList);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            zone_destroy(zc, array[i].zone);
        }
        
        array_destroy(zc->zoneList);
        zc->zoneList = NULL;
    }
}

void zc_main_loop(R(ZC*) zc)
{
    R(UdpSocket*) socket = zc->socket;
    
    for (;;)
    {
        udp_socket_recv(socket);
        
        db_thread_execute_query_callbacks(core_db_thread(C(zc)));
        timer_pool_execute_callbacks(&zc->timerPool);
        
        udp_socket_send(socket);
        udp_socket_check_timeouts(socket);
        
        if (zc_ipc_check(zc))
        {
            log_format(B(zc), LogInfo, "Shutting down cleanly");
            break;
        }
        
        clock_sleep_milliseconds(25);
    }
}

void zc_start_zone(R(ZC*) zc, int sourceId)
{
    ZoneBySourceId zone;
    int zoneId;
    int instId;
    
    // Make sure the sourceId is valid
    zoneId = sourceId % EQP_SOURCE_ID_ZONE_INSTANCE_OFFSET;
    instId = sourceId / EQP_SOURCE_ID_ZONE_INSTANCE_OFFSET;
    
    if (instId > EQP_ZONE_MAX_INSTANCE_ID)
        return;
    
    //fixme: check zoneId max?
    if (zoneId < 1)
        return;
    
    // Make sure this zone isn't already running
    if (zc_get_zone_by_source_id(zc, sourceId))
        return;
    
    // Create our new zone
    zone.sourceId   = sourceId;
    zone.zone       = zone_create(zc, sourceId, zoneId, instId);
    
    array_push_back(B(zc), &zc->zoneList, &zone);
    
    // Tell the log writer to open a log for this zone / instance
    ipc_buffer_write(B(zc), shm_viewer_memory_type(&zc->shmViewerLogWriter, IpcBuffer), ServerOp_LogOpen, sourceId, 0, NULL);
}

Zone* zc_get_zone_by_source_id(R(ZC*) zc, int sourceId)
{
    R(ZoneBySourceId*) array    = array_data_type(zc->zoneList, ZoneBySourceId);
    uint32_t n                  = array_count(zc->zoneList);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        if (array[i].sourceId == sourceId)
            return array[i].zone;
    }
    
    return NULL;
}

/* LuaJIT API */

void zc_log(R(ZC*) zc, R(const char*) str)
{
    log_format(B(zc), LogLua, "%s", str);
}

void zc_log_for(R(ZC*) zc, int sourceId, R(const char*) str)
{
    log_from_format(B(zc), sourceId, LogLua, "%s", str);
}
