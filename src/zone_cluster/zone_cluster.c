
#include "zone_cluster.h"

void zc_init(R(ZC*) zc, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath, R(const char*) sourceId, R(const char*) port)
{
    zc->sourceId = strtol(sourceId, NULL, 10) + EQP_SOURCE_ID_ZONE_CLUSTER_OFFSET;
    
    timer_pool_init(B(zc), &zc->timerPool);
    
    ipc_set_open(B(zc), &zc->ipcSet, zc->sourceId, ipcPath, masterIpcPath, logWriterIpcPath);

    core_init(C(zc), zc->sourceId, ipc_set_log_writer_ipc(&zc->ipcSet));
    
    // Containers
    zc->zoneList                = array_create_type(B(zc), ZoneBySourceId);
    zc->expectedClientsByName   = hash_table_create_type(B(zc), Client*);
    
    // Lua
    zc_lua_init(zc);
    
    // UDP socket
    zc->socket = udp_socket_create(B(zc));
    udp_socket_open(zc->socket, strtol(port, NULL, 10));
}

void zc_deinit(R(ZC*) zc)
{
    core_deinit(C(zc));
    ipc_set_deinit(&zc->ipcSet);
    
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
    
    if (zc->expectedClientsByName)
    {
        hash_table_destroy(zc->expectedClientsByName);
        zc->expectedClientsByName = NULL;
    }
}

void zc_main_loop(R(ZC*) zc)
{
    R(UdpSocket*) socket    = zc->socket;
    R(IpcSet*) ipcSet       = &zc->ipcSet;
    int sourceId            = zc->sourceId;
    
    for (;;)
    {
        udp_socket_recv(socket);
        
        db_thread_execute_query_callbacks(core_db_thread(C(zc)));
        timer_pool_execute_callbacks(&zc->timerPool);
        
        udp_socket_send(socket);
        udp_socket_check_timeouts(socket);
        
        if (ipc_set_receive(B(zc), ipcSet))
        {
            log_format(B(zc), LogInfo, "Shutting down cleanly");
            break;
        }
        
        ipc_set_keep_alive(B(zc), ipcSet, sourceId);
        
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
    
    if (instId < 0 || instId > EQP_SOURCE_ID_ZONE_INSTANCE_MAX)
        return;
    
    if (zoneId < 1 || zoneId > EQP_SOURCE_ID_ZONE_MAX)
        return;
    
    // Make sure this zone isn't already running
    if (zc_get_zone_by_source_id(zc, sourceId))
        return;
    
    // Create our new zone
    zone.sourceId   = sourceId;
    zone.zone       = zone_create(zc, sourceId, zoneId, instId);
    
    array_push_back(B(zc), &zc->zoneList, &zone);
    
    // Tell the log writer to open a log for this zone / instance
    ipc_set_log_file_open(B(zc), &zc->ipcSet, sourceId);
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

static void zc_expected_client_timeout_callback(R(Timer*) timer)
{
    R(Client*) client   = timer_userdata_type(timer, Client);
    R(ZC*) zc           = client_zone_cluster(client);
    R(String*) name     = client_name(client);
    
    if (hash_table_get_by_str(zc->expectedClientsByName, name))
    {
        hash_table_remove_by_str(zc->expectedClientsByName, name);
        client_drop(client);
    }
    
    timer_destroy(timer);
}

void zc_client_expected_to_zone_in(R(ZC*) zc, int sourceId, R(IpcPacket*) packet)
{
    R(Server_ClientZoning*) zoning;
    R(Zone*) zone;
    R(Client*) client;
    
    if (ipc_packet_length(packet) < sizeof(Server_ClientZoning))
        return;
    
    zoning  = ipc_packet_data_type(packet, Server_ClientZoning);
    zone    = zc_get_zone_by_source_id(zc, sourceId);
    
    if (!zone)
        return;
    
    client = client_create(zc, zone, zoning);
    
    /*
        This timer is not explicitly stored anywhere: it is safe for its callback to execute
        when the client is not timing out, since they will be taken out of the expectedClient
        list first; and the timer triggers its destruction from the callback.
    */
    eqp_timer_create(B(zc), zc_timer_pool(zc), EQP_CLIENT_ZONE_IN_EXPECTED_TIMEOUT, zc_expected_client_timeout_callback, client, true);
    
    hash_table_set_by_cstr(B(zc), &zc->expectedClientsByName, zoning->characterName, strlen(zoning->characterName), (void*)&client);
}

void zc_client_match_with_expected(R(ZC*) zc, R(Client*) clientStub, R(ProtocolHandler*) handler, R(const char*) name)
{
    // Working with the assumption that the IPC notification of this zone in (as handled by zc_client_expected_to_zone_in() above)
    // will always occur before the client connects and sends enough UDP packets to trigger this call.
    uint32_t len        = strlen(name);
    R(Client**) pclient = hash_table_get_type_by_cstr(zc->expectedClientsByName, name, len, Client*);
    
    if (pclient)
    {
        R(Client*) client = *pclient;
        
        hash_table_remove_by_cstr(zc->expectedClientsByName, name, len);
        
        client_set_handler(client, handler);
        client_catch_up_with_loading_progress(client);
    }
    else
    {
        protocol_handler_drop(handler);
    }
    
    free(clientStub);
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
