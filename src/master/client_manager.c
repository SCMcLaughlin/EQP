
#include "client_manager.h"
#include "master_ipc.h"

static void client_mgr_motd_callback(R(Query*) query)
{
    R(ClientMgr*) mgr               = query_userdata_type(query, ClientMgr);
    R(MasterIpcThread*) ipcThread   = mgr->ipcThread;
    
    atomic_mutex_lock(&mgr->mutex);
    
    while (query_select(query))
    {
        uint32_t len;
        R(const char*) motd     = query_get_string(query, 1, &len);
        mgr->messageOfTheDay    = string_create_from_cstr(B(ipcThread), motd, len);
    }
    
    atomic_mutex_unlock(&mgr->mutex);
}

static void client_mgr_base_time_callback(R(Query*) query)
{
    R(ClientMgr*) mgr = query_userdata_type(query, ClientMgr);
    
    atomic_mutex_lock(&mgr->mutex);
    
    while (query_select(query))
    {
        mgr->eqTimeBaseUnixSeconds = query_get_int64(query, 1);
    }
    
    atomic_mutex_unlock(&mgr->mutex);
}

void client_mgr_init(R(MasterIpcThread*) ipcThread, R(ClientMgr*) mgr, R(lua_State*) L)
{
    Query query;
    R(Database*) db = master_ipc_thread_db(ipcThread);
    
    mgr->ipcThread      = ipcThread;
    mgr->clientsByName  = hash_table_create_type(B(ipcThread), Client*);
    mgr->clients        = array_create_type(B(ipcThread), ClientByIds);
    
    atomic_mutex_lock(&mgr->mutex);
    
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
    
    atomic_mutex_unlock(&mgr->mutex);

    lua_sys_run_file(B(ipcThread), L, EQP_MASTER_LOGIN_CONFIG, 1);
    mgr->remoteAddress  = lua_sys_field_to_string(B(ipcThread), L, -1, "remoteaddress");
    mgr->localAddress   = lua_sys_field_to_string(B(ipcThread), L, -1, "localaddress");
    lua_pop(L, 1);
    
    atomic_mutex_lock(&mgr->mutex);
    zc_mgr_init(ipcThread, &mgr->zoneClusterMgr, L);
    atomic_mutex_unlock(&mgr->mutex);
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

uint64_t client_mgr_eq_time_base_unix_seconds(R(ClientMgr*) mgr)
{
    uint64_t time;
    atomic_mutex_lock(&mgr->mutex);
    time = mgr->eqTimeBaseUnixSeconds;
    atomic_mutex_unlock(&mgr->mutex);
    return time;
}

String* client_mgr_message_of_the_day(R(ClientMgr*) mgr)
{
    R(String*) str;
    atomic_mutex_lock(&mgr->mutex);
    str = mgr->messageOfTheDay;
    atomic_mutex_unlock(&mgr->mutex);
    return str;
}

void client_mgr_handle_zone_in_from_char_select(R(ClientMgr*) mgr, R(IpcPacket*) packet)
{
    R(Server_ClientZoning*) zoning;
    R(Database*) db;
    Query query;
    (void)mgr;
    (void)packet;
    
    if (ipc_packet_length(packet) < sizeof(Server_ClientZoning))
        return;
    
    zoning  = (Server_ClientZoning*)ipc_packet_data(packet);
    db      = client_mgr_db(mgr);
}
