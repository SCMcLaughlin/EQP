
#ifndef EQP_CLIENT_MANAGER_H
#define EQP_CLIENT_MANAGER_H

#include "define.h"
#include "eqp_string.h"
#include "eqp_array.h"
#include "eqp_hash_table.h"
#include "server_structs.h"
#include "client.h"
#include "database.h"
#include "query.h"
#include "atomic_mutex.h"
#include "lua_sys.h"
#include "ipc_buffer.h"
#include "zone_cluster_manager.h"

#define EQP_MASTER_LOGIN_CONFIG "scripts/master/login_config_loader.lua"

STRUCT_DECLARE(MasterIpcThread);

STRUCT_DEFINE(ClientMgr)
{
    /*
        The master process has one actively-used locking primitive, and this is it.
        Whenever the main thread is touching its ChildProcesses, it owns the lock.
        Whenever the ipc thread is doing ANYTHING, it owns the lock.
        Whenever the main thread needs to do one-off things that touch stuff belonging
        to the ipc thread (db callbacks, etc), the it will lock on those.
    */
    AtomicMutex     mutex;
    uint64_t        eqTimeBaseUnixSeconds;
    String*         remoteAddress;
    String*         localAddress;
    String*         messageOfTheDay;
    
    HashTable*      clientsByName;
    Array*          clients;
    
    ZoneClusterMgr  zoneClusterMgr;
    
    MasterIpcThread*    ipcThread;
};

void        client_mgr_init(MasterIpcThread* ipcThread, ClientMgr* mgr, lua_State* L);
void        client_mgr_deinit(ClientMgr* mgr);

void        client_mgr_handle_zone_in_from_char_select(ClientMgr* mgr, IpcPacket* packet);

#define     client_mgr_get_or_start_zone_cluster(mgr, sourceId) zc_mgr_get_or_start(&(mgr)->zoneClusterMgr, (sourceId))
#define     client_mgr_get_zone_cluster(mgr, sourceId) zc_mgr_get(&(mgr)->zoneClusterMgr, (sourceId))

#define     client_mgr_db(mgr) master_ipc_thread_db((mgr)->ipcThread)
#define     client_mgr_lock_init(mgr) atomic_mutex_init(&(mgr)->mutex)
#define     client_mgr_lock(mgr) atomic_mutex_lock(&(mgr)->mutex)
#define     client_mgr_unlock(mgr) atomic_mutex_unlock(&(mgr)->mutex)

#endif//EQP_CLIENT_MANAGER_H
