
#ifndef EQP_ZONE_CLUSTER_H
#define EQP_ZONE_CLUSTER_H

#include "define.h"
#include "eqp_core.h"
#include "eqp_log.h"
#include "eqp_clock.h"
#include "eqp_array.h"
#include "eqp_hash_table.h"
#include "source_id.h"
#include "server_op.h"
#include "ipc_set.h"
#include "server_structs.h"
#include "timer_pool.h"
#include "timer.h"
#include "zone.h"
#include "client.h"
#include "udp_socket.h"
#include "lua_sys.h"
#include "lua_object.h"
#include "zone_cluster_ipc.h"

STRUCT_DEFINE(ZC)
{
    // Core MUST be the first member of this struct
    Core        core;
    
    lua_State*  L;
    TimerPool   timerPool;
    UdpSocket*  socket;
    
    Array*      zoneList;
    HashTable*  expectedClientsByName;  // Clients that haven't connected yet, but are expected to soon
    //Array*      clientList;
    //Array*      npcList;
    
    int         sourceId;
    IpcSet      ipcSet;
};

void    zc_init(R(ZC*) zc, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath, R(const char*) sourceId, R(const char*) port);
void    zc_deinit(R(ZC*) zc);
void    zc_main_loop(R(ZC*) zc);

#define zc_lua(zc) ((zc)->L)
#define zc_timer_pool(zc) (&(zc)->timerPool)

void    zc_start_zone(R(ZC*) zc, int sourceId);
Zone*   zc_get_zone_by_source_id(R(ZC*) zc, int sourceId);

void    zc_client_expected_to_zone_in(R(ZC*) zc, int sourceId, R(IpcPacket*) packet);
void    zc_client_match_with_expected(R(ZC*) zc, R(Client*) clientStub, R(ProtocolHandler*) handler, R(const char*) name);

/* LuaJIT API */
EQP_API void    zc_log(R(ZC*) zc, R(const char*) str);
EQP_API void    zc_log_for(R(ZC*) zc, int sourceId, R(const char*) str);

#endif//EQP_ZONE_CLUSTER_H
