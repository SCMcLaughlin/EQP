
#include "client_manager.h"
#include "master_ipc.h"
#include "eqp_master.h"

static void client_mgr_motd_callback(R(Query*) query)
{
    R(ClientMgr*) mgr               = query_userdata_type(query, ClientMgr);
    R(MasterIpcThread*) ipcThread   = mgr->ipcThread;
    
    client_mgr_lock(mgr);
    
    while (query_select(query))
    {
        uint32_t len;
        R(const char*) motd     = query_get_string(query, 1, &len);
        mgr->messageOfTheDay    = string_create_from_cstr(B(ipcThread), motd, len);
    }
    
    client_mgr_unlock(mgr);
}

static void client_mgr_base_time_callback(R(Query*) query)
{
    R(ClientMgr*) mgr = query_userdata_type(query, ClientMgr);
    
    client_mgr_lock(mgr);
    
    while (query_select(query))
    {
        mgr->eqTimeBaseUnixSeconds = query_get_int64(query, 1);
    }
    
    client_mgr_unlock(mgr);
}

void client_mgr_init(R(MasterIpcThread*) ipcThread, R(ClientMgr*) mgr, R(lua_State*) L)
{
    Query query;
    R(Database*) db = master_ipc_thread_db(ipcThread);
    
    mgr->ipcThread      = ipcThread;
    mgr->clientsByName  = hash_table_create_type(B(ipcThread), Client*);
    mgr->clients        = array_create_type(B(ipcThread), ClientByIds);
    
    client_mgr_lock(mgr);
    
    query_init(&query);
    query_set_userdata(&query, mgr);
    db_prepare_literal(db, &query, "SELECT value FROM parameter WHERE key = ?", client_mgr_motd_callback);
    query_bind_string_literal(&query, 1, "messageOfTheDay");
    db_schedule(db, &query);
    
    query_init(&query);
    query_set_userdata(&query, mgr);
    db_prepare_literal(db, &query, "SELECT value FROM parameter WHERE key = ?", client_mgr_base_time_callback);
    query_bind_string_literal(&query, 1, "eqBaseUnixSeconds");
    db_schedule(db, &query);
    
    client_mgr_unlock(mgr);

    lua_sys_run_file(B(ipcThread), L, EQP_MASTER_LOGIN_CONFIG, 1);
    mgr->remoteAddress  = lua_sys_field_to_string(B(ipcThread), L, -1, "remoteaddress");
    mgr->localAddress   = lua_sys_field_to_string(B(ipcThread), L, -1, "localaddress");
    lua_pop(L, 1);
    
    client_mgr_lock(mgr);
    zc_mgr_init(ipcThread, &mgr->zoneClusterMgr, L);
    client_mgr_unlock(mgr);
}

void client_mgr_deinit(R(ClientMgr*) mgr)
{
    if (mgr->remoteAddress)
    {
        string_destroy(mgr->remoteAddress);
        mgr->remoteAddress = NULL;
    }
    
    if (mgr->localAddress)
    {
        string_destroy(mgr->localAddress);
        mgr->localAddress = NULL;
    }
    
    if (mgr->messageOfTheDay)
    {
        string_destroy(mgr->messageOfTheDay);
        mgr->messageOfTheDay = NULL;
    }
    
    if (mgr->clientsByName)
    {
        hash_table_destroy(mgr->clientsByName);
        mgr->clientsByName = NULL;
    }
    
    if (mgr->clients)
    {
        array_destroy(mgr->clients);
        mgr->clients = NULL;
    }
    
    zc_mgr_deinit(&mgr->zoneClusterMgr);
}

static void client_mgr_ipc_char_select(R(ClientMgr*) mgr, ServerOp opcode, uint32_t length, R(const void*) data)
{
    R(MasterIpcThread*) ipcThread   = mgr->ipcThread;
    R(ChildProcess*) proc           = master_get_child_process(master_ipc_thread_master(ipcThread), EQP_SOURCE_ID_CHAR_SELECT);
    
    proc_ipc_send(B(ipcThread), proc, opcode, EQP_SOURCE_ID_MASTER, length, data);
}

static void client_mgr_add_client_from_char_select(R(ClientMgr*) mgr, R(Client*) client)
{
    byte buf[EQP_IPC_PACKET_MAX_SIZE];
    R(Server_ZoneAddress*) zoneAddr = (Server_ZoneAddress*)buf;
    R(Basic*) basic                 = B(mgr->ipcThread);
    R(String*) motd                 = mgr->messageOfTheDay;
    uint32_t motdLen                = string_length(motd);
    uint32_t length                 = sizeof(Server_ZoneAddress);
    int zoneSourceId                = client_zone_source_id(client);
    ClientByIds byIds;
    R(ZoneCluster*) zc;
    
    if ((motdLen + length) > sizeof(buf))
        motdLen = sizeof(buf) - length;
    
    length += motdLen;
    
    // Retrieve the zone cluster we are sending the client to (starting the zone and/or the zone cluster if needed)
    
    zc = client_mgr_get_or_start_zone_cluster(mgr, zoneSourceId);
    client_set_zone_cluster(client, zc);
    zone_cluster_inform_of_client_zoning_in(zc, client, zoneSourceId);
    
    // Write and send the zone address packet to char select
    
    zoneAddr->accountId             = client_account_id(client);
    snprintf(zoneAddr->ipAddress, sizeof(zoneAddr->ipAddress), "%s", client_is_local(client) ? string_data(mgr->localAddress) : string_data(mgr->remoteAddress));
    zoneAddr->portHostByteOrder     = zone_cluster_port(zc);
    snprintf(zoneAddr->shortName, sizeof(zoneAddr->shortName), "%s", zone_short_name_by_id(client_zone_id(client)));
    zoneAddr->eqTimeBaseUnixSeconds = mgr->eqTimeBaseUnixSeconds;
    zoneAddr->motdLength            = motdLen;
    snprintf(zoneAddr->messageOfTheDay, motdLen, "%s", string_data(mgr->messageOfTheDay));
    
    client_mgr_ipc_char_select(mgr, ServerOp_ZoneAddress, length, buf);
    
    // Insert the client into the client mgr's lists
    
    byIds.characterId   = client_character_id(client);
    byIds.accountId     = client_account_id(client);
    byIds.ipAddress     = client_ip_address(client);
    byIds.client        = client;
    
    array_push_back(basic, &mgr->clients, &byIds);
    
    hash_table_set_by_cstr(basic, &mgr->clientsByName, client_name(client), strlen(client_name(client)), (void*)&client);
}

static void client_mgr_from_char_select_callback(R(Query*) query)
{
    R(Client*) client   = query_userdata_type(query, Client);
    R(ClientMgr*) mgr   = client_client_mgr(client);
    int isValid         = false;
    
    client_mgr_lock(mgr);
    
    while (query_select(query))
    {
        int64_t charId  = query_get_int64(query, 1);
        int zoneId      = query_get_int(query, 2);
        int instId      = query_get_int(query, 3);
        
        client_set_character_id(client, charId);
        client_set_zone_id(client, zoneId);
        client_set_instance_id(client, instId);
        
        client_mgr_add_client_from_char_select(mgr, client);
        
        isValid = true;
    }
    
    if (!isValid)
    {
        Server_ClientZoningReject reject;
        
        reject.accountId = client_account_id(client);
        snprintf(reject.zoneShortName, sizeof(reject.zoneShortName), "qeynos");
        
        client_mgr_ipc_char_select(mgr, ServerOp_ClientZoningReject, sizeof(reject), &reject);
        client_destroy(client);
    }
    
    client_mgr_unlock(mgr);
}

void client_mgr_handle_zone_in_from_char_select(R(ClientMgr*) mgr, R(IpcPacket*) packet)
{
    R(Server_ClientZoning*) zoning;
    R(Database*) db;
    R(Client*) client;
    Query query;
    
    if (ipc_packet_length(packet) < sizeof(Server_ClientZoning))
        return;
    
    /*
        At this point, we have:
        * accountId
        * accountName
        * characterName
        * ipAddress
        * isLocal
    
        We need to verify that this characterName does belong to that accountId,
        and also retrieve the characterId and the target zoneId and instanceId to
        send them to.
    */
    
    zoning  = (Server_ClientZoning*)ipc_packet_data(packet);
    client  = client_create(B(mgr->ipcThread), mgr, zoning);
    db      = client_mgr_db(mgr);
    
    query_init(&query);
    query_set_userdata(&query, client);
    
    db_prepare_literal(db, &query, 
        "SELECT character_id, zone_id, instance_id FROM character "
        "WHERE name_id_pair = ? AND name = ?", client_mgr_from_char_select_callback);
    
    query_bind_int64(&query, 1, (int64_t)zoning->accountId);
    query_bind_string(&query, 2, zoning->characterName, QUERY_CALC_LENGTH);
    
    db_schedule(db, &query);
}
