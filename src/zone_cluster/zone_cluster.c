
#include "zone_cluster.h"

void zc_init(R(ZC*) zc, R(const char*) ipcPath, R(const char*) masterIpcPath, R(const char*) logWriterIpcPath, R(const char*) sourceId, R(const char*) port)
{
    (void)ipcPath;
    (void)masterIpcPath;
    
    zc->sourceId = strtol(sourceId, NULL, 10) + EQP_SOURCE_ID_ZONE_CLUSTER_OFFSET;
    
    timer_pool_init(B(zc), &zc->timerPool);
    
    shm_viewer_init(&zc->shmViewerLogWriter);
    shm_viewer_open(B(zc), &zc->shmViewerLogWriter, logWriterIpcPath, sizeof(IpcBuffer));
    // Tell the log writer to open our log file
    ipc_buffer_write(B(zc), shm_viewer_memory_type(&zc->shmViewerLogWriter, IpcBuffer), ServerOpLogOpen, zc->sourceId, 0, NULL);
    
    core_init(C(zc), zc->sourceId, shm_viewer_memory_type(&zc->shmViewerLogWriter, IpcBuffer));
    
    zc_lua_init(zc);
    
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
        
        clock_sleep_milliseconds(25);
    }
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
