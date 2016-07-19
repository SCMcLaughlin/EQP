
#include "zone_cluster.h"

void zc_init(ZC* zc, const char* ipcPath, const char* masterIpcPath, const char* logWriterIpcPath, const char* sourceId, const char* port)
{
    zc->sourceId = strtol(sourceId, NULL, 10) + EQP_SOURCE_ID_ZONE_CLUSTER_OFFSET;
    
    timer_pool_init(B(zc), &zc->timerPool);
    
    ipc_set_open(B(zc), &zc->ipcSet, zc->sourceId, ipcPath, masterIpcPath, logWriterIpcPath);

    core_init(C(zc), zc->sourceId, ipc_set_log_writer_ipc(&zc->ipcSet));
    
    // Containers
    zc->zoneList                = array_create_type(B(zc), ZoneBySourceId);
    zc->expectedClientsByName   = hash_table_create_type(B(zc), Client*);
    zc->connectedClients        = array_create_type(B(zc), ConnectedClient);
    zc->connectedClientsByName  = hash_table_create_type(B(zc), Client*);
    
    // Lua
    zc_lua_init(zc);
    
    // UDP socket
    zc->socket = udp_socket_create(B(zc));
    udp_socket_open(zc->socket, strtol(port, NULL, 10));
    
    // Etc
    item_share_mem_init(&zc->items);
    
    // Always-running timers
    timer_init(&zc->timerClientKeepAlive, &zc->timerPool, EQP_ZC_CLIENT_KEEP_ALIVE_DELAY_MS, zc_client_keep_alive_callback, zc, true);
}

void zc_deinit(ZC* zc)
{
    core_deinit(C(zc));
    ipc_set_deinit(&zc->ipcSet);
    
    item_share_mem_close(&zc->items);
    
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
        ZoneBySourceId* array   = array_data_type(zc->zoneList, ZoneBySourceId);
        uint32_t n              = array_count(zc->zoneList);
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
    
    if (zc->connectedClients)
    {
        array_destroy(zc->connectedClients);
        zc->connectedClients = NULL;
    }
    
    if (zc->connectedClientsByName)
    {
        hash_table_destroy(zc->connectedClientsByName);
        zc->connectedClientsByName = NULL;
    }
}

void zc_main_loop(ZC* zc)
{
    UdpSocket* socket   = zc->socket;
    IpcSet* ipcSet      = &zc->ipcSet;
    int sourceId        = zc->sourceId;
    
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

void zc_start_zone(ZC* zc, int sourceId)
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
    
    // Tell the log writer to open a log for this zone / instance
    ipc_set_log_file_open(B(zc), &zc->ipcSet, sourceId);
    
    // Create our new zone
    zone.sourceId   = sourceId;
    zone.zone       = zone_create(zc, sourceId, zoneId, instId);
    
    array_push_back(B(zc), &zc->zoneList, &zone);
}

Zone* zc_get_zone_by_source_id(ZC* zc, int sourceId)
{
    ZoneBySourceId* array   = array_data_type(zc->zoneList, ZoneBySourceId);
    uint32_t n              = array_count(zc->zoneList);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        if (array[i].sourceId == sourceId)
            return array[i].zone;
    }
    
    return NULL;
}

static void zc_expected_client_timeout_callback(Timer* timer)
{
    Client* client  = timer_userdata_type(timer, Client);
    ZC* zc          = client_zone_cluster(client);
    String* name    = client_name(client);

    if (hash_table_get_by_str(zc->expectedClientsByName, name))
    {
        hash_table_remove_by_str(zc->expectedClientsByName, name);
        client_drop(client);
    }
    
    timer_destroy(timer);
}

void zc_client_expected_to_zone_in(ZC* zc, int sourceId, IpcPacket* packet)
{
    Server_ClientZoning* zoning;
    Zone* zone;
    Client* client;
    
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

void zc_client_match_with_expected(ZC* zc, Client* clientStub, ProtocolHandler* handler, const char* name)
{
    // Working with the assumption that the IPC notification of this zone in (as handled by zc_client_expected_to_zone_in() above)
    // will always occur before the client connects and sends enough UDP packets to trigger this call.
    uint32_t len        = strlen(name);
    Client** pclient    = hash_table_get_type_by_cstr(zc->expectedClientsByName, name, len, Client*);
    
    if (pclient)
    {
        Client* client = *pclient;
        
        hash_table_remove_by_cstr(zc->expectedClientsByName, name, len);
        
        client_set_handler(client, handler);
        client->expansion = clientStub->expansion;
        protocol_handler_set_client_object(handler, client);
        
        client_catch_up_with_loading_progress(client);
    }
    else
    {
        protocol_handler_drop(handler);
    }
    
    free(clientStub);
}

void zc_client_keep_alive_callback(Timer* timer)
{
    ZC* zc                  = timer_userdata_type(timer, ZC);
    ConnectedClient* array  = array_data_type(zc->connectedClients, ConnectedClient);
    uint32_t n              = array_count(zc->connectedClients);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        ConnectedClient* cli = &array[i];
        
        // If the ProtocolHandler is NULL, this client must be linkdead
        if (!cli->handler)
            continue;
        
        if (cli->expansion == ExpansionId_Trilogy)
            client_trilogy_send_keep_alive(cli->handler);
        /*else
            client_standard_send_keep_alive(cli->handler);*/
    }
}

void zc_add_connected_client(ZC* zc, Client* client)
{
    String* name = client_name(client);
    Client** prev;
    ConnectedClient con;
    
    // Is this client already connected?
    prev = hash_table_get_by_str(zc->connectedClientsByName, name);
    
    if (prev)
    {
        Client* cli = *prev;
        zc_remove_connected_client(zc, cli, false);
        
        // If the client is linkdead, it will be dropped by the pending linkdead timeout timer
        if (!client_is_linkdead(cli))
        {
            client_forcibly_disconnect(cli);
            client_drop(cli);
        }
    }
    
    // Insert into our zone-cluster-wide lists
    hash_table_set_by_str(B(zc), &zc->connectedClientsByName, name, (void*)&client);
    
    con.expansion   = client_expansion(client);
    con.handler     = client_handler(client);
    con.client      = client;
    
    array_push_back(B(zc), &zc->connectedClients, &con);
    
    zone_spawn_client(zc, client_zone(client), client);
}

static void zc_remove_linkdead_client_callback(Timer* timer)
{
    Client* client  = timer_userdata_type(timer, Client);
    ZC* zc          = client_zone_cluster(client);
    
    zc_remove_connected_client(zc, client, false);
    
    client_drop(client);
    timer_destroy(timer);
}

void zc_remove_connected_client(ZC* zc, Client* client, int isLinkdead)
{
    ConnectedClient* array  = array_data_type(zc->connectedClients, ConnectedClient);
    uint32_t n              = array_count(zc->connectedClients);
    String* name;
    uint32_t i;
    
    if (isLinkdead)
    {
        client_grab(client);
        protocol_handler_drop(client_handler(client));
        client_set_handler(client, NULL);
        client_set_linkdead(client);
        
        zone_mark_client_as_linkdead(client_zone(client), client);
        
        for (i = 0; i < n; i++)
        {
            if (array[i].client == client)
            {
                array[i].handler = NULL;
                break;
            }
        }
        
        eqp_timer_create(B(zc), zc_timer_pool(zc), EQP_ZC_CLIENT_LINKDEAD_LINGER_TIMEOUT, zc_remove_linkdead_client_callback, client, true);
        return;
    }
    
    name = client_name(client);
    
    for (i = 0; i < n; i++)
    {
        if (array[i].client == client)
        {
            zone_remove_client(client_zone(client), client);
            hash_table_remove_by_str(zc->connectedClientsByName, name);
            array_swap_and_pop(zc->connectedClients, i);
            break;
        }
    }
}

void zc_items_open(ZC* zc, IpcPacket* packet)
{
    Server_ItemSharedMemoryOpen* open;

    if (ipc_packet_length(packet) < (sizeof(Server_ItemSharedMemoryOpen) + 1))
        return;
    
    open = ipc_packet_data_type(packet, Server_ItemSharedMemoryOpen);
    
    item_share_mem_open(B(zc), zc_lua(zc), &zc->items, open->path, open->length);
}

void zc_zone_log_format(ZC* zc, Zone* zone, LogType type, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_from_vformat(B(zc), zone_get_source_id(zone), type, fmt, args);
    va_end(args);
}

/* LuaJIT API */

void zc_log(ZC* zc, const char* str)
{
    log_format(B(zc), LogLua, "%s", str);
}

void zc_log_for(ZC* zc, int sourceId, const char* str)
{
    log_from_format(B(zc), sourceId, LogLua, "%s", str);
}
