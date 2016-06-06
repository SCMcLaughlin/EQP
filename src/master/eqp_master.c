
#include "eqp_master.h"
#include "eqp_clock.h"

static void child_proc_init(R(ChildProcess*) proc)
{
    proc->ipc                   = NULL;
    proc->lastActivityTimestamp = 0;
    proc->pid                   = 0;
    shm_viewer_init(&proc->shmViewer);
}

void master_init(R(Master*) M)
{
    atomic_mutex_init(&M->mutexProc);
    
    master_ipc_thread_init(M, &M->ipcThread);
    
    child_proc_init(&M->procCharSelect);
    child_proc_init(&M->procLogWriter);
    child_proc_init(&M->procLogin);
    
    // Create IPC shared memory buffer for master, need to know its path before launching child processes
    ipc_buffer_shm_create_init(B(M), &master_ipc_thread_ipc_buffer(&M->ipcThread), &M->shmCreatorMaster, &M->shmViewerMaster, EQP_MASTER_SHM_PATH);
    
    // Create the IPC shared memory buffer for the log writer process early
    ipc_buffer_shm_create_init(B(M), &M->procLogWriter.ipc, &M->procLogWriter.shmCreator, &M->procLogWriter.shmViewer, EQP_LOG_SHM_PATH);
    
    core_init(C(M), EQP_SOURCE_ID_MASTER, M->procLogWriter.ipc);
}

void master_deinit(R(Master*) M)
{
    master_shut_down_all_child_processes(M);
    
    core_deinit(C(M));
    
    share_mem_destroy(&M->shmCreatorMaster, &M->shmViewerMaster);
    share_mem_destroy(&M->procLogWriter.shmCreator, &M->procLogWriter.shmViewer);
}

static void master_shut_down_child_process(R(Master*) M, R(ChildProcess*) proc)
{
    if (!proc->ipc)
        return;
    
    ipc_buffer_write(B(M), proc->ipc, ServerOpShutdown, EQP_SOURCE_ID_MASTER, 0, NULL);
    proc->ipc = NULL;
}

void master_shut_down_all_child_processes(R(Master*) M)
{
    // Shut down the log writer last
    master_shut_down_child_process(M, &M->procLogWriter);
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
        const char* argv[] = {path, arg1, arg2, NULL};
        
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
    // The log writer does not need to know about the master process or the log writer (obviously), its communication is all input only
    M->procLogWriter.pid = master_spawn_process(M, "./eqp-log-writer", share_mem_path(&M->procLogWriter.shmCreator), NULL, NULL);
    
    // Tell the log writer to open the log file for Master
    ipc_buffer_write(B(M), M->procLogWriter.ipc, ServerOpLogOpen, EQP_SOURCE_ID_MASTER, 0, NULL);
    
    M->procLogWriter.lastActivityTimestamp = clock_milliseconds();
}

static void master_start_process(R(Master*) M, R(const char*) binPath, R(ChildProcess*) proc, R(const char*) ipcPath)
{
    ExceptionScope exScope;
    
    switch (exception_try(B(M), &exScope))
    {
    case Try:
        atomic_mutex_lock(&M->mutexProc);
        ipc_buffer_shm_create_init(B(M), &proc->ipc, &proc->shmCreator, &proc->shmViewer, ipcPath);
    
        proc->pid = master_spawn_process(M, binPath, share_mem_path(&proc->shmCreator),
            share_mem_path(&M->shmCreatorMaster), share_mem_path(&M->procLogWriter.shmCreator));
    
        proc->lastActivityTimestamp = clock_milliseconds();
        break;
    
    case Finally:
        atomic_mutex_unlock(&M->mutexProc);
        break;
    }
    
    exception_end_try_with_finally(B(M));
}

void master_start_char_select(R(Master*) M)
{
    (void)M;
    //master_start_process(M, "./eqp-char-select", &M->procCharSelect, EQP_CHAR_SELECT_SHM_PATH);
}

void master_start_login(R(Master*) M)
{
    (void)M;
    //master_start_process(M, "./eqp-login", &M->procLogin, EQP_LOGIN_SHM_PATH);
}

ChildProcess* master_get_child_process(R(Master*) M, int sourceId)
{
    ChildProcess* proc = NULL;
    
    atomic_mutex_lock(&M->mutexProc);
    
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
    
    atomic_mutex_unlock(&M->mutexProc);
    
    return proc;
}

static void master_check_threads_status(R(Master*) M)
{
    if (!thread_is_running(T(&M->ipcThread)))
        exception_throw_message(B(M), ErrorThread, "[master_check_threads_status] IPC thread lifetime ended unexpectedly", 0);
}

static void master_check_child_procs_status(R(Master*) M)
{
    (void)M;
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
