
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

#define EQP_MASTER_LOGIN_CONFIG "scripts/master/login_config_loader.lua"

STRUCT_DECLARE(MasterIpcThread);

STRUCT_DEFINE(ClientMgr)
{
    AtomicMutex mutex;
    uint64_t    eqTimeBaseUnixSeconds;
    String*     remoteAddress;
    String*     localAddress;
    String*     messageOfTheDay;
    
    HashTable*  clientsByName;
    Array*      clients;
    
    MasterIpcThread*    ipcThread;
};

void        client_mgr_init(R(MasterIpcThread*) ipcThread, R(ClientMgr*) mgr, R(lua_State*) L);
void        client_mgr_deinit(R(ClientMgr*) mgr);

uint64_t    client_mgr_eq_time_base_unix_seconds(R(ClientMgr*) mgr);
String*     client_mgr_message_of_the_day(R(ClientMgr*) mgr);

#endif//EQP_CLIENT_MANAGER_H
