
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
#include "item_share_mem.h"

#define EQP_ZC_CLIENT_KEEP_ALIVE_DELAY_MS       500
#define EQP_ZC_CLIENT_LINKDEAD_LINGER_TIMEOUT   TIMER_SECONDS(30)

STRUCT_DEFINE(ZC)
{
    // Core MUST be the first member of this struct
    Core            core;
    
    lua_State*      L;
    TimerPool       timerPool;
    UdpSocket*      socket;
    
    Array*          zoneList;
    HashTable*      expectedClientsByName;  // Clients that haven't connected yet, but are expected to soon
    Array*          connectedClients;
    HashTable*      connectedClientsByName;
    //Array*          npcList;
    
    ItemShareMem    items;
    
    int             sourceId;
    IpcSet          ipcSet;
    
    Timer           timerClientKeepAlive;
};

STRUCT_DEFINE(ConnectedClient)
{
    int                 expansion;
    ProtocolHandler*    handler;
    Client*             client;
};

void    zc_init(ZC* zc, const char* ipcPath, const char* masterIpcPath, const char* logWriterIpcPath, const char* sourceId, const char* port);
void    zc_deinit(ZC* zc);
void    zc_main_loop(ZC* zc);

#define zc_lua(zc) ((zc)->L)
#define zc_timer_pool(zc) (&(zc)->timerPool)

void    zc_start_zone(ZC* zc, int sourceId);
Zone*   zc_get_zone_by_source_id(ZC* zc, int sourceId);

void    zc_client_expected_to_zone_in(ZC* zc, int sourceId, IpcPacket* packet);
void    zc_client_match_with_expected(ZC* zc, Client* clientStub, ProtocolHandler* handler, const char* name);

void    zc_client_keep_alive_callback(Timer* timer);
void    zc_add_connected_client(ZC* zc, Client* client);
void    zc_remove_connected_client(ZC* zc, Client* client, int isLinkdead);

void    zc_items_open(ZC* zc, IpcPacket* packet);
#define zc_item_by_id(zc, itemId) item_share_mem_get_prototype(&(zc)->items, (itemId))

void    zc_zone_log_format(ZC* zc, Zone* zone, LogType type, const char* fmt, ...);

/* LuaJIT API */
EQP_API void    zc_log(ZC* zc, const char* str);
EQP_API void    zc_log_for(ZC* zc, int sourceId, const char* str);

#endif//EQP_ZONE_CLUSTER_H
