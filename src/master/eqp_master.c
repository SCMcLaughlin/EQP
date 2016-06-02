
#include "eqp_master.h"

static void child_proc_init(R(ChildProcess*) proc)
{
    proc->ipc = NULL;
    proc->pid = 0;
    shm_viewer_init(&proc->shmViewer);
}

void master_init(R(Master*) M)
{
    child_proc_init(&M->procCharSelect);
    child_proc_init(&M->procLogWriter);
    child_proc_init(&M->procLogin);
    
    // Create the IPC shared memory buffer for the log writer process early
    share_mem_create(B(M), &M->procLogWriter.shmCreator, &M->procLogWriter.shmViewer, EQP_LOG_SHM_PATH, sizeof(IpcBuffer));
    M->procLogWriter.ipc = shm_viewer_memory_type(&M->procLogWriter.shmViewer, IpcBuffer);
    ipc_buffer_init(B(M), M->procLogWriter.ipc);
    
    core_init(C(M), EQP_SOURCE_ID_MASTER, M->procLogWriter.ipc);
}

void master_deinit(R(Master*) M)
{
    master_shut_down_all_child_processes(M);
    
    core_deinit(C(M));
    
    share_mem_destroy(&M->procLogWriter.shmCreator, &M->procLogWriter.shmViewer);
}

static void master_shut_down_child_process(R(Master*) M, R(ChildProcess*) proc)
{
    if (!proc->ipc)
        return;
    
    ipc_buffer_write(B(M), proc->ipc, ServerOpShutdown, EQP_SOURCE_ID_MASTER, 0, NULL);
}

void master_shut_down_all_child_processes(R(Master*) M)
{
    // Shut down the log writer last
    master_shut_down_child_process(M, &M->procLogWriter);
}

static pid_t master_spawn_process(R(Master*) M, R(const char*) path, R(const char*) arg1, R(const char*) arg2)
{
    pid_t pid;
    
    log_format(B(M), LogInfo, "Spawning process \"%s\" with args \"%s\", \"%s\"", path, arg1 ? arg1 : "(null)", arg2 ? arg2 : "(null)");
    
    pid = fork();
    
    if (pid == 0)
    {
        const char* argv[] = {path, arg1, arg2, NULL};
        
        if (execv(path, (char**)argv))
        {
            // This will be called in the context of the the forked Master child process.. yeah
            printf("[master_spawn_process] child process execv() failed attempting to execute '%s', aborting", path);
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
    M->procLogWriter.pid = master_spawn_process(M, "./eqp-log-writer", share_mem_path(&M->procLogWriter.shmCreator), NULL);
    
    // Tell the log writer to open the log file for Master
    ipc_buffer_write(B(M), M->procLogWriter.ipc, ServerOpLogOpen, EQP_SOURCE_ID_MASTER, 0, NULL);
}

void master_start_char_select(R(Master*) M)
{
    M->procCharSelect.pid = master_spawn_process(M, "./eqp-char-select", share_mem_path(&M->procCharSelect.shmCreator), share_mem_path(&M->procLogWriter.shmCreator));
}

void master_start_login(R(Master*) M)
{
    M->procLogin.pid = master_spawn_process(M, "./eqp-login", share_mem_path(&M->procLogin.shmCreator), share_mem_path(&M->procLogWriter.shmCreator));
}
