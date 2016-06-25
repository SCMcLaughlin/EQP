
#include "eqp_master.h"

#define BIN_LOG_WRITER      "./eqp-log-writer"
#define BIN_LOGIN           "./eqp-login"
#define BIN_CHAR_SELECT     "./eqp-char-select"
#define BIN_ZONE_CLUSTER    "./eqp-zone-cluster"

void master_init(R(Master*) M)
{
    master_lock_init(M);
    
    timer_pool_init(B(M), &M->timerPool);
    timer_init(&M->timerStatusChecks, &M->timerPool, EQP_STATUS_CHECK_FREQUENCY, master_status_checks_callback, M, true);
    
    master_ipc_thread_init(M, &M->ipcThread);
    
    proc_init(&M->procCharSelect);
    proc_init(&M->procLogWriter);
    proc_init(&M->procLogin);
    
    M->zoneClusterProcs = array_create_type(B(M), ZoneCluster*);
    
    // Create IPC shared memory buffer for master, need to know its path before launching child processes
    ipc_buffer_shm_create_init(B(M), &master_ipc_thread_ipc_buffer(&M->ipcThread), &M->shmCreatorMaster, &M->shmViewerMaster, EQP_MASTER_SHM_PATH);
    
    // Create the IPC shared memory buffer for the log writer process early
    proc_create_ipc_buffer(B(M), &M->procLogWriter, EQP_LOG_WRITER_SHM_PATH);
    
    core_init(C(M), EQP_SOURCE_ID_MASTER, M->procLogWriter.ipc);
}

void master_deinit(R(Master*) M)
{
    master_shut_down_all_child_processes(M);
    
    if (M->zoneClusterProcs)
    {
        array_destroy(M->zoneClusterProcs);
        M->zoneClusterProcs = NULL;
    }
    
    timer_pool_deinit(&M->timerPool);
    
    core_deinit(C(M));
    
    share_mem_destroy(&M->shmCreatorMaster, &M->shmViewerMaster);
}

void master_shut_down_all_child_processes(R(Master*) M)
{
    R(ZoneCluster**) zcProcs    = array_data_type(M->zoneClusterProcs, ZoneCluster*);
    uint32_t n                  = array_count(M->zoneClusterProcs);
    uint32_t i;
    
    master_lock(M);
    
    proc_shutdown(M, &M->procLogin);
    proc_shutdown(M, &M->procCharSelect);
    
    for (i = 0; i < n; i++)
    {
        proc_shutdown(M, zone_cluster_proc(zcProcs[i]));
    }
    
    // Shut down the log writer last, and give other processes some time to do any last logging
    clock_sleep_milliseconds(250);
    proc_shutdown(M, &M->procLogWriter);

    master_unlock(M);
}

static pid_t master_spawn_process(R(Master*) M, R(const char*) path, R(const char*) arg1, R(const char*) arg2, R(const char*) arg3,
    R(const char*) arg4, R(const char*) arg5)
{
    pid_t pid;
    
    /*
        Most child processes take 3 arguments:
        1) path to their own shared mem ipc input buffer
        2) path to the eqp-master ipc buffer
        3) path to the eqp-log-writer ipc buffer
    
        eqp-zone-cluster takes two more:
        4) id (index) number for this zone cluster
        5) UDP port for this zone cluster
    */
    
    log_format(B(M), LogInfo, "Spawning process \"%s\" with args \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
        path, arg1 ? arg1 : "(null)", arg2 ? arg2 : "(null)", arg3 ? arg3 : "(null)", arg4 ? arg4 : "(null)", arg5 ? arg5 : "(null)");
    
    pid = fork();
    
    if (pid == 0)
    {
        const char* argv[] = {path, arg1, arg2, arg3, arg4, arg5, NULL};
        
        if (execv(path, (char**)argv))
        {
            // This will be called in the context of the the forked Master child process.. yeah
            printf(TERM_RED "[master_spawn_process] child process execv() failed attempting to execute '%s', aborting\n" TERM_DEFAULT, path);
            abort();
        }
    }
    else if (pid < 0)
    {
        // Fork failed
        exception_throw_message(B(M), ErrorSpawnProcess, "[master_spawn_process] fork() failed", 0);
    }
    
    log_format(B(M), LogInfo, "Spawned process \"%s\" with pid %i", path, pid);
    
    return pid;
}

void master_start_log_writer(R(Master*) M)
{
    // Tell the log writer to open the log file for Master once it's started
    ipc_buffer_write(B(M), proc_ipc(&M->procLogWriter), ServerOp_LogOpen, EQP_SOURCE_ID_MASTER, 0, NULL);
    
    // The log writer does not need to know about the master process or the log writer (obviously), its communication is all input only
    proc_start(&M->procLogWriter, master_spawn_process(M, BIN_LOG_WRITER, proc_shm_path(&M->procLogWriter), NULL, NULL, NULL, NULL));
}

static void master_start_process(R(Master*) M, R(const char*) binPath, R(ChildProcess*) proc, R(const char*) ipcPath, R(const char*) id, R(const char*) port)
{
    ExceptionScope exScope;
    
    switch (exception_try(B(M), &exScope))
    {
    case Try:
        proc_create_ipc_buffer(B(M), proc, ipcPath);
        proc_start(proc, master_spawn_process(M, binPath, proc_shm_path(proc), share_mem_path(&M->shmCreatorMaster), proc_shm_path(&M->procLogWriter), id, port));
        break;
    
    default:
        //fixme: log this
        exception_handled(B(M));
        break;
    }
    
    exception_end_try(B(M));
}

void master_start_char_select(R(Master*) M)
{
    //master_start_process(M, BIN_CHAR_SELECT, &M->procCharSelect, EQP_CHAR_SELECT_SHM_PATH, NULL, NULL);
    
    //temp:
    proc_create_ipc_buffer(B(M), &M->procCharSelect, EQP_CHAR_SELECT_SHM_PATH);
}

void master_start_login(R(Master*) M)
{
    master_start_process(M, BIN_LOGIN, &M->procLogin, EQP_LOGIN_SHM_PATH, NULL, NULL);
}

void master_start_zone_cluster(R(Master*) M, R(ZoneCluster*) zoneCluster, uint16_t id, uint16_t port)
{
    char idBuf[32];
    char portBuf[32];
    
    snprintf(idBuf, sizeof(idBuf), "%u", id);
    snprintf(portBuf, sizeof(portBuf), "%u", port);
    
    master_start_process(M, BIN_ZONE_CLUSTER, zone_cluster_proc(zoneCluster), EQP_ZONE_CLUSTER_SHM_PATH, idBuf, portBuf);
    
    array_push_back(B(M), &M->zoneClusterProcs, (void*)&zoneCluster);
}

ChildProcess* master_get_child_process(R(Master*) M, int sourceId)
{
    ChildProcess* proc = NULL;
    
    // Only the IPC thread calls this function; it owns the lock whenever it calls this.

    switch (sourceId)
    {
    case EQP_SOURCE_ID_LOG_WRITER:
        proc = &M->procLogWriter;
        break;
    
    case EQP_SOURCE_ID_LOGIN:
        proc = &M->procLogin;
        break;
    
    case EQP_SOURCE_ID_CHAR_SELECT:
        proc = &M->procCharSelect;
        break;
    
    default:
        break;
    }

    return proc;
}

static void master_check_threads_status(R(Master*) M)
{
    if (!thread_is_running(T(&M->ipcThread)))
        exception_throw_message(B(M), ErrorThread, "[master_check_threads_status] IPC thread lifetime ended unexpectedly", 0);
}

static void master_restart_proc(R(Master*) M, R(ChildProcess*) proc, int sourceId, R(const char*) binPath, R(const char*) ipcPath,
    R(const char*) id, R(const char*) port)
{
    log_from_format(B(M), sourceId, LogInfo, "eqp-master has detected that this process may be non-responsive; restarting...");
    log_format(B(M), LogInfo, "Detected that process %i for binary '%s' is non-responsive; restarting it", proc_pid(proc), binPath);
    
    // Tell log-writer to close the log for this process
    ipc_buffer_write(B(M), proc_ipc(&M->procLogWriter), ServerOp_LogClose, sourceId, 0, NULL);
    
    proc_kill(proc);
    
    master_start_process(M, binPath, proc, ipcPath, id, port);
}

static void master_check_child_procs_status(R(Master*) M)
{
    R(ZoneCluster**) zcProcs    = array_data_type(M->zoneClusterProcs, ZoneCluster*);
    uint32_t n                  = array_count(M->zoneClusterProcs);
    uint64_t time               = clock_milliseconds();
    uint32_t i;
    
    if ((time - proc_last_activity_time(&M->procLogin)) >= EQP_CHILD_PROC_TIMEOUT_MILLISECONDS)
        master_restart_proc(M, &M->procLogin, EQP_SOURCE_ID_LOGIN, BIN_LOGIN, EQP_LOGIN_SHM_PATH, NULL, NULL);

    //if ((time - proc_last_activity_time(&M->procCharSelect)) >= EQP_CHILD_PROC_TIMEOUT_MILLISECONDS)
    //    master_restart_proc(M, &M->procCharSelect, EQP_SOURCE_ID_CHAR_SELECT, BIN_CHAR_SELECT, EQP_CHAR_SELECT_SHM_PATH, NULL, NULL);
    
    (void)zcProcs;
    (void)n;
    (void)i;
    /*
    for (i = 0; i < n; i++)
    {
        R(ZoneCluster*) zc = &zcProcs[i];
    
        if ((time - proc_last_activity_time(zone_cluster_proc(zc))) >= EQP_CHILD_PROC_TIMEOUT_MILLISECONDS)
        {
            char idBuf[32];
            char portBuf[32];
    
            snprintf(idBuf, sizeof(idBuf), "%u", id);
            snprintf(portBuf, sizeof(portBuf), "%u", port);
    
            master_restart_proc(M, zone_cluster_proc(zc), zone_cluster_source_id(zc), BIN_ZONE_CLUSTER, EQP_ZONE_CLUSTER_SHM_PATH, idBuf, portBuf);
            //fixme: client_mgr needs to be informed
        }
    }
    */
}

void master_status_checks_callback(R(Timer*) timer)
{
    R(Master*) M = timer_userdata_type(timer, Master);
    
    master_lock(M);
    
    master_check_threads_status(M);
    master_check_child_procs_status(M);
    
    master_unlock(M);
}

void master_main_loop(R(Master*) M)
{
    for (;;)
    {
        // Have we received the shutdown signal?
        if (master_ipc_thread_shutdown_received(&M->ipcThread))
        {
            log_format(B(M), LogInfo, "Shutting down cleanly");
            break;
        }
        
        db_thread_execute_query_callbacks(core_db_thread(C(M)));
        timer_pool_execute_callbacks(&M->timerPool);
        
        clock_sleep_milliseconds(50);
    }
}

#undef BIN_LOG_WRITER
#undef BIN_LOGIN
#undef BIN_CHAR_SELECT
#undef BIN_ZONE_CLUSTER
