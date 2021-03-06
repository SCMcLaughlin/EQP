
#ifndef EQP_MASTER_H
#define EQP_MASTER_H

#include "define.h"
#include "eqp_core.h"
#include "eqp_log.h"
#include "source_id.h"
#include "ipc_buffer.h"
#include "share_mem.h"
#include "atomic_mutex.h"
#include "master_ipc.h"
#include "eqp_clock.h"
#include "child_process.h"
#include "timer_pool.h"
#include "timer.h"
#include "eqp_array.h"
#include "zone_cluster.h"
#include "item_shm_loader.h"

#define EQP_MASTER_SHM_PATH                 "shm/eqp-master-"
#define EQP_LOG_WRITER_SHM_PATH             "shm/eqp-log-writer-"
#define EQP_LOGIN_SHM_PATH                  "shm/eqp-login-"
#define EQP_CHAR_SELECT_SHM_PATH            "shm/eqp-char-select-"
#define EQP_ZONE_CLUSTER_SHM_PATH           "shm/eqp-zone-cluster-"
#define EQP_CHILD_PROC_TIMEOUT_MILLISECONDS TIMER_SECONDS(15)
#define EQP_STATUS_CHECK_FREQUENCY          TIMER_SECONDS(3)

/*
    Master is the first server process to run and the last to shut down.
    It has a few mostly-related jobs:
    
    1) Managing all other server processes and their lifetimes
    2) Managing all shared memory (including IPC buffers for other processes)
    3) Centralizing IPC between the other server processes (except eqp-login and eqp-log-writer)
    4) Responding to Console commands
    5) Being aware of which zones are running and which ZoneCluster each is being handled by
    6) Being aware of all active clients, their present status and location (in terms of which process is talking to them)
    7) Managing active groups and raids
    8) Routing inter-process client requests (zoning, chat) and maintaining a per-client chat queue when messages can't be delivered
    
    To do all this, it uses 3 threads:
    
    1) The main thread, which handles periodic and timed events (mainly detecting when other processes disappear or become non-responsive)
    2) The IPC thread, which handles requests from other processes, including Console commands
    3) The database thread (mostly for group and raid management and Console commands, but also generating shared memory for e.g. items)
    
    Threads 2 and 3 will spend most of their time blocking on semaphores. 
*/

STRUCT_DEFINE(Master)
{
    // Core MUST be the first member of this struct
    Core            core;
    
    TimerPool       timerPool;
    
    // Master IPC
    MasterIpcThread ipcThread;
    ShmViewer       shmViewerMaster;
    ShmCreator      shmCreatorMaster;
    
    // Item shared memory
    ItemShmLoader   items;
    
    // Child processes and associated shared memory
    ChildProcess    procCharSelect;
    ChildProcess    procLogWriter;
    ChildProcess    procLogin;
    Array*          zoneClusterProcs;
    
    Timer           timerStatusChecks;
};

void            master_init(Master* M);
void            master_deinit(Master* M);
void            master_shut_down_all_child_processes(Master* M);

#define         master_start_ipc_thread(M) master_ipc_thread_start(B(M), &(M)->ipcThread)

void            master_main_loop(Master* M);

void            master_start_log_writer(Master* M);
void            master_start_char_select(Master* M);
void            master_start_login(Master* M);
void            master_start_zone_cluster(Master* M, ZoneCluster* zoneCluster, uint16_t id, uint16_t port);

void            master_status_checks_callback(Timer* timer);

ChildProcess*   master_get_child_process(Master* M, int sourceId);

#define         master_db(M) core_db(C((M)))
#define         master_lock_init(M) master_ipc_thread_lock_init(&(M)->ipcThread)
#define         master_lock(M) master_ipc_thread_lock(&(M)->ipcThread)
#define         master_unlock(M) master_ipc_thread_unlock(&(M)->ipcThread)

#endif//EQP_MASTER_H
