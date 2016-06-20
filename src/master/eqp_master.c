
#include "eqp_master.h"

#define BIN_LOG_WRITER  "./eqp-log-writer"
#define BIN_LOGIN       "./eqp-login"
#define BIN_CHAR_SELECT "./eqp-char-select"

void master_init(R(Master*) M)
{
    atomic_mutex_init(&M->mutexProcList);
    
    master_ipc_thread_init(M, &M->ipcThread);
    
    proc_init(&M->procCharSelect);
    proc_init(&M->procLogWriter);
    proc_init(&M->procLogin);
    
    // Create IPC shared memory buffer for master, need to know its path before launching child processes
    ipc_buffer_shm_create_init(B(M), &master_ipc_thread_ipc_buffer(&M->ipcThread), &M->shmCreatorMaster, &M->shmViewerMaster, EQP_MASTER_SHM_PATH);
    
    // Create the IPC shared memory buffer for the log writer process early
    proc_create_ipc_buffer(M, &M->procLogWriter, EQP_LOG_WRITER_SHM_PATH);
    
    core_init(C(M), EQP_SOURCE_ID_MASTER, M->procLogWriter.ipc);
}

void master_deinit(R(Master*) M)
{
    master_shut_down_all_child_processes(M);
    
    core_deinit(C(M));
    
    share_mem_destroy(&M->shmCreatorMaster, &M->shmViewerMaster);
}

void master_shut_down_all_child_processes(R(Master*) M)
{
    atomic_mutex_lock(&M->mutexProcList);
    
    proc_shutdown(M, &M->procLogin);
    proc_shutdown(M, &M->procCharSelect);
    
    // Shut down the log writer last, and give other processes some time to do any last logging
    clock_sleep_milliseconds(250);
    proc_shutdown(M, &M->procLogWriter);
    
    atomic_mutex_unlock(&M->mutexProcList);
}

static pid_t master_spawn_process(R(Master*) M, R(const char*) path, R(const char*) arg1, R(const char*) arg2, R(const char*) arg3)
{
    pid_t pid;
    
    /*
        Most child processes take 3 arguments:
        1) path to their own shared mem ipc input buffer
        2) path to the eqp-master ipc buffer
        3) path to the eqp-log-writer ipc buffer
    
        Any other startup 'arguments' can be sent via IPC
    */
    
    log_format(B(M), LogInfo, "Spawning process \"%s\" with args \"%s\", \"%s\", \"%s\"",
        path, arg1 ? arg1 : "(null)", arg2 ? arg2 : "(null)", arg3 ? arg3 : "(null)");
    
    pid = fork();
    
    if (pid == 0)
    {
        const char* argv[] = {path, arg1, arg2, arg3, NULL};
        
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
    ipc_buffer_write(B(M), proc_ipc(&M->procLogWriter), ServerOpLogOpen, EQP_SOURCE_ID_MASTER, 0, NULL);
    
    // The log writer does not need to know about the master process or the log writer (obviously), its communication is all input only
    proc_start(&M->procLogWriter, master_spawn_process(M, BIN_LOG_WRITER, proc_shm_path(&M->procLogWriter), NULL, NULL));
}

static void master_start_process(R(Master*) M, R(const char*) binPath, R(ChildProcess*) proc, R(const char*) ipcPath)
{
    ExceptionScope exScope;
    
    switch (exception_try(B(M), &exScope))
    {
    case Try:
        atomic_mutex_lock(&M->mutexProcList);
        proc_create_ipc_buffer(M, proc, ipcPath);
        proc_start(proc, master_spawn_process(M, binPath, proc_shm_path(proc), share_mem_path(&M->shmCreatorMaster), proc_shm_path(&M->procLogWriter)));
        break;
    
    case Finally:
        atomic_mutex_unlock(&M->mutexProcList);
        break;
    }
    
    exception_end_try_with_finally(B(M));
}

void master_start_char_select(R(Master*) M)
{
    (void)M;
    //master_start_process(M, BIN_CHAR_SELECT, &M->procCharSelect, EQP_CHAR_SELECT_SHM_PATH);
}

void master_start_login(R(Master*) M)
{
    master_start_process(M, BIN_LOGIN, &M->procLogin, EQP_LOGIN_SHM_PATH);
}

ChildProcess* master_get_child_process(R(Master*) M, int sourceId)
{
    ChildProcess* proc = NULL;
    
    atomic_mutex_lock(&M->mutexProcList);
    
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
    
    atomic_mutex_unlock(&M->mutexProcList);
    
    return proc;
}

static void master_check_threads_status(R(Master*) M)
{
    if (!thread_is_running(T(&M->ipcThread)))
        exception_throw_message(B(M), ErrorThread, "[master_check_threads_status] IPC thread lifetime ended unexpectedly", 0);
}

static void master_restart_proc(R(Master*) M, R(ChildProcess*) proc, int sourceId, R(const char*) binPath, R(const char*) ipcPath)
{
    log_from_format(B(M), sourceId, LogInfo, "eqp-master has detected that this process may be non-responsive; restarting...");
    log_format(B(M), LogInfo, "Detected that process %i for binary '%s' is non-responsive; restarting it", proc_pid(proc), binPath);
    
    // Tell log-writer to close the log for this process
    ipc_buffer_write(B(M), proc_ipc(&M->procLogWriter), ServerOpLogClose, sourceId, 0, NULL);
    
    proc_kill(proc);
    
    master_start_process(M, binPath, proc, ipcPath);
}

static void master_check_child_procs_status(R(Master*) M)
{
    uint64_t time = clock_milliseconds();
    
    if ((time - proc_last_activity_time(&M->procLogin)) >= EQP_CHILD_PROC_TIMEOUT_MILLISECONDS)
        master_restart_proc(M, &M->procLogin, EQP_SOURCE_ID_LOGIN, BIN_LOGIN, EQP_LOGIN_SHM_PATH);
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
        
        // Check that threads are still running
        master_check_threads_status(M);
        
        // Check whether any child processes have timed out or disappeared
        master_check_child_procs_status(M);
        
        clock_sleep_milliseconds(1000);
    }
}

#undef BIN_LOG_WRITER
#undef BIN_LOGIN
#undef BIN_CHAR_SELECT
