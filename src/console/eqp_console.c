
#include "eqp_console.h"

STRUCT_DEFINE(ConsoleCommand)
{
    uint32_t    argCount;
    uint32_t    optionCount;
    byte        data[0];
};

void console_init(Console* console)
{
    basic_init(B(console), EQP_SOURCE_ID_CONSOLE, NULL);
    
    console->ipcSend = NULL;
    shm_viewer_init(&console->shmViewerMaster);
    
    ipc_buffer_shm_create_init(B(console), &console->ipcRecv, &console->shmCreatorConsole, &console->shmViewerConsole, EQP_CONSOLE_SHM_PATH);
}

void console_deinit(Console* console)
{
    basic_deinit(B(console));
    
    shm_viewer_close(&console->shmViewerMaster);
    share_mem_destroy(&console->shmCreatorConsole, &console->shmViewerConsole);
}

static int console_command_is_start(int argc, const char** argv)
{
    int i;
    
    for (i = 1; i < argc; i++)
    {
        const char* arg = argv[i];
        
        if (*arg == '-')
            continue;
        
        return (strcmp(arg, "start") == 0);
    }
    
    return false;
}

static int console_command_has_force_option(int argc, const char** argv)
{
    int i;
    
    for (i = 1; i < argc; i++)
    {
        const char* arg = argv[i];
        
        if (*arg != '-')
            continue;
        
        while (*arg == '-') arg++;
        
        if (strcmp(arg, "f") == 0 || strcmp(arg, "force") == 0)
            return true;
    }
    
    return false;
}

#define CONSOLE_ERR_BADDIR          "Directory 'shm' does not exist!"
#define CONSOLE_ERR_NO_MASTER       "eqp-master process is not running. It must be running to accept commands.\nUse 'eqp start' to start the eqp-master process."
#define CONSOLE_ERR_MASTER_LAUNCH   "failed to launch eqp-master process"
#define CONSOLE_ERR_TIMEOUT         "timed out waiting for a response."
#define CONSOLE_ERR_ARG_LENGTH      "too many arguments provided."
#define CONSOLE_ERR_MASTER_ALREADY  "eqp-master process already appears to be running.\nIf it previously crashed, this may be a false positive.\nUse the -f or -force option if you believe this is the case."
#define CONSOLE_MASTER_SHM          "eqp-master-"
#define CONSOLE_ANY_SHM             "eqp-"

static void console_launch_master_process(Basic* basic)
{
    pid_t pid = fork();
    
    if (pid == 0)
    {
        const char* argv[] = {"./eqp-master", NULL};
        
        if (execv("./eqp-master", (char**)argv))
        {
            // This will be called in the context of the the forked Master process.. yeah
            printf(TERM_RED "[console_launch_master_process] child process execv() failed attempting to execute './eqp-master', aborting\n" TERM_DEFAULT);
            abort();
        }
    }
    else if (pid < 0)
    {
        // Fork failed
        exception_throw_literal(basic, ErrorConsole, CONSOLE_ERR_MASTER_LAUNCH);
    }
}

static void console_master_ipc_open(Console* console, const char* name)
{
    char path[512];
    snprintf(path, sizeof(path), "shm/%s", name);
    
    shm_viewer_open(B(console), &console->shmViewerMaster, path, sizeof(IpcBuffer));
    console->ipcSend = shm_viewer_memory_type(&console->shmViewerMaster, IpcBuffer);
}

static int console_find_master_ipc(Console* console)
{
#ifdef EQP_WINDOWS
    
#else
    DIR* shmDir = opendir("shm");
    int ret     = false;
    struct dirent entry;
    struct dirent* result;
    
    if (!shmDir)
        exception_throw_literal(B(console), ErrorConsole, CONSOLE_ERR_BADDIR);
    
    while (!readdir_r(shmDir, &entry, &result))
    {
        if (result == NULL)
            break;
        
        if (strncmp(entry.d_name, CONSOLE_MASTER_SHM, sizeof(CONSOLE_MASTER_SHM) - 1) == 0)
        {
            console_master_ipc_open(console, entry.d_name);
            ret = true;
            break;
        }
    }

    closedir(shmDir);
    return ret;
#endif
}

static void console_force_start_master(Console* console)
{
#ifdef EQP_WINDOWS
    
#else
    DIR* shmDir = opendir("shm");
    struct dirent entry;
    struct dirent* result;
    
    if (!shmDir)
        exception_throw_literal(B(console), ErrorConsole, CONSOLE_ERR_BADDIR);
    
    while (!readdir_r(shmDir, &entry, &result))
    {
        if (result == NULL)
            break;
        
        if (strncmp(entry.d_name, CONSOLE_ANY_SHM, sizeof(CONSOLE_ANY_SHM) - 1) == 0 && !strstr(entry.d_name, "console"))
        {
            char path[512];
            snprintf(path, sizeof(path), "shm/%s", entry.d_name);
            remove(path);
        }
    }

    closedir(shmDir);
#endif
    
    console_launch_master_process(B(console));
}

static void console_add_arg(Console* console, byte* data, const char* arg, uint32_t* length)
{
    uint32_t len = strlen(arg);
    uint32_t pos = *length;
    
    *length += sizeof(uint32_t) + len + 1;
    
    if (*length > EQP_IPC_PACKET_MAX_SIZE)
        exception_throw_literal(B(console), ErrorConsole, CONSOLE_ERR_ARG_LENGTH);
    
    memcpy(data + pos, &len, sizeof(uint32_t));
    len++; // Include null terminator
    memcpy(data + pos + sizeof(uint32_t), arg, len);
}

static void console_do_send(Console* console, int argc, const char** argv)
{
    byte data[EQP_IPC_PACKET_MAX_SIZE];
    uint32_t length     = sizeof(ConsoleCommand);
    ConsoleCommand* cmd = (ConsoleCommand*)data;
    int i;
    
    memset(data, 0, EQP_IPC_PACKET_MAX_SIZE);
    
    // The first string is always the path to the console's ipc buffer
    console_add_arg(console, data, share_mem_path(&console->shmCreatorConsole), &length);
        
    // Arguments
    for (i = 1; i < argc; i++)
    {
        const char* arg = argv[i];
        
        if (*arg == '-')
            continue;
        
        console_add_arg(console, data, arg, &length);
        cmd->argCount++;
    }
    
    // Options
    for (i = 1; i < argc; i++)
    {
        const char* opt = argv[i];
        
        if (*opt != '-')
            continue;
        
        while (*opt == '-') opt++;
        
        console_add_arg(console, data, opt, &length);
        cmd->optionCount++;
    }
    
    ipc_buffer_write(B(console), console->ipcSend, ServerOp_ConsoleCommand, EQP_SOURCE_ID_CONSOLE, length, data);
}

static int console_wait_start(Console* console)
{
    clock_sleep_milliseconds(1000);
    
    if (!console_find_master_ipc(console))
    {
        printf("eqp-master startup seems to have failed...\n");
        return false;
    }
    
    return true;
}

int console_send(Console* console, int argc, const char** argv)
{
    int isStart = console_command_is_start(argc, argv);
    
    if (!console_find_master_ipc(console))
    {
        if (!isStart)
            exception_throw_literal(B(console), ErrorConsole, CONSOLE_ERR_NO_MASTER);
        
        printf("Attempting to start eqp-master...\n");
        console_launch_master_process(B(console));
        
        if (!console_wait_start(console))
            return false;
    }
    else if (isStart)
    {
        if (console_command_has_force_option(argc, argv))
        {
            printf("Attempting to force-start eqp-master...\n");
            console_force_start_master(console);
            
            if (console_wait_start(console))
                goto do_send;
        }
        
        exception_throw_literal(B(console), ErrorConsole, CONSOLE_ERR_MASTER_ALREADY);
    }
    
do_send:
    console_do_send(console, argc, argv);
    return true;
}

static void console_write_packet(IpcPacket* packet)
{
    uint32_t i;
    uint32_t n = ipc_packet_length(packet);
    char* data = (char*)ipc_packet_data(packet);
    
    // Don't assume result is null-terminated...
    for (i = 0; i < n; i++)
    {
        char c = data[i];
        
        if (c)
            fputc(c, stdout);
    }
    
    fputc('\n', stdout);
}

void console_recv(Console* console)
{
    uint32_t time   = clock_milliseconds();
    int run         = true;
    
    while (run)
    {
        IpcPacket packet;
        
        if ((clock_milliseconds() - time) >= EQP_CONSOLE_TIMEOUT_MILLISECONDS)
            exception_throw_literal(B(console), ErrorConsole, CONSOLE_ERR_TIMEOUT);
        
        if (ipc_buffer_read(B(console), console->ipcRecv, &packet))
        {
            switch (ipc_packet_opcode(&packet))
            {
            case ServerOp_ConsoleMessage:
                console_write_packet(&packet);
                time = clock_milliseconds();
                break;
            
            case ServerOp_ConsoleFinish:
                run = false;
                break;
                
            default:
                break;
            }
            
            ipc_packet_deinit(&packet);
        }
        else
        {
            clock_sleep_milliseconds(10);
        }
    }
}
