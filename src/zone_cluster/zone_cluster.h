
#ifndef EQP_ZONE_CLUSTER_H
#define EQP_ZONE_CLUSTER_H

#include "define.h"
#include "eqp_core.h"
#include "eqp_log.h"
#include "eqp_clock.h"
#include "eqp_array.h"
#include "source_id.h"
#include "server_op.h"
#include "ipc_buffer.h"
#include "share_mem.h"
#include "timer_pool.h"
#include "timer.h"
#include "zone.h"
#include "client.h"
#include "client_packet_trilogy.h"
#include "client_packet_standard.h"
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
    Array*      clientList;
    Array*      npcList;
    
    int         sourceId;
    IpcBuffer*  ipc;
    IpcBuffer*  ipcMaster;
    ShmViewer   shmViewerSelf;
    ShmViewer   shmViewerMaster;
    ShmViewer   shmViewerLogWriter;
};

void    zc_init(R(ZC*) zc, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath, R(const char*) sourceId, R(const char*) port);
void    zc_deinit(R(ZC*) zc);
void    zc_main_loop(R(ZC*) zc);

/* LuaJIT API */
EQP_API void    zc_log(R(ZC*) zc, R(const char*) str);
EQP_API void    zc_log_for(R(ZC*) zc, int sourceId, R(const char*) str);

#endif//EQP_ZONE_CLUSTER_H
