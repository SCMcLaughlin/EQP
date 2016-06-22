
#include "master_ipc.h"
#include "eqp_master.h"

void master_ipc_thread_init(R(Master*) M, R(MasterIpcThread*) ipcThread)
{
    atomic_mutex_init(&ipcThread->mutexShutdown);
    atomic_mutex_lock(&ipcThread->mutexShutdown);
    ipcThread->master = M;
}

void master_ipc_thread_console_reply(R(MasterIpcThread*) ipcThread, R(IpcBuffer*) ipc, R(const char*) src, R(const char*) msg)
{
    if (!src)
    {
        ipc_buffer_write(B(ipcThread), ipc, ServerOp_ConsoleMessage, EQP_SOURCE_ID_MASTER, strlen(msg) + 1, msg);
    }
    else
    {
        char buf[EQP_IPC_PACKET_MAX_SIZE];
        int len1 = strlen(src) + 1;
        int len2 = strlen(msg) + 1;
        
        memcpy(buf, src, len1);
        memcpy(buf + len1, msg, len2);
        
        ipc_buffer_write(B(ipcThread), ipc, ServerOp_ConsoleMessage, EQP_SOURCE_ID_MASTER, len1 + len2, buf);
    }
}

void master_ipc_thread_console_finish(R(MasterIpcThread*) ipcThread, R(IpcBuffer*) ipc, R(const char*) src)
{
    ipc_buffer_write(B(ipcThread), ipc, ServerOp_ConsoleFinish, EQP_SOURCE_ID_MASTER, src ? (strlen(src) + 1) : 0, src);
}

static uint32_t master_ipc_thread_console_message_push_string(R(MasterIpcThread*) ipcThread, R(lua_State*) L, R(byte*) data, uint32_t pos, const uint32_t length)
{
    uint32_t* arglen = (uint32_t*)(data + pos);
    pos += sizeof(uint32_t);
    
    if (pos < length)
    {
        const char* str = (const char*)(data + pos);
        pos += *arglen + 1;
        
        if (pos <= length)
        {
            lua_pushlstring(L, str, *arglen);
            return pos;
        }
    }
    
    exception_throw_format(B(ipcThread), ErrorIpc, "[master_ipc_thread_console_message_push_string] Invalid argument string length: at position %u out of %u", pos, length);
    return 0; // Unreachable
}

static uint32_t master_ipc_thread_console_message_table(R(MasterIpcThread*) ipcThread, R(lua_State*) L, R(byte*) data, uint32_t pos, const uint32_t length, uint32_t count)
{
    uint32_t i;
    
    lua_createtable(L, count, 0);
    
    for (i = 1; i <= count; i++)
    {
        lua_pushinteger(L, i);
        pos = master_ipc_thread_console_message_push_string(ipcThread, L, data, pos, length);
        lua_settable(L, -3);
    }
    
    return pos;
}

static int master_ipc_thread_handle_console_message(R(MasterIpcThread*) ipcThread, R(Master*) M, R(lua_State*) L, R(IpcPacket*) packet, R(ChildProcess*) proc, int skipSourceArg)
{
    uint32_t length     = ipc_packet_length(packet);
    byte* data          = ipc_packet_data(packet);
    uint32_t argCount   = *(uint32_t*)(data + 0);
    uint32_t optCount   = *(uint32_t*)(data + sizeof(uint32_t));
    uint32_t pos        = sizeof(uint32_t) * 2;
    IpcBuffer* ipc      = proc->ipc;
    int ret             = false;
    
    lua_getglobal(L, "handle_console");
    if (!lua_isfunction(L, -1))
        exception_throw_message(B(ipcThread), ErrorLua, "[master_ipc_thread_handle_console_message] Lua interface 'handle_console' is undefined", 0);
    
    // handle_console(thread, M, ipc, src, cmd, args, opts)
    
    //fixme: figure out how the Lua system should work for C objects
    lua_pushlightuserdata(L, ipcThread);
    lua_pushlightuserdata(L, M);
    lua_pushlightuserdata(L, ipc);
    
    // src
    if (skipSourceArg)
    {
        uint32_t* arglen = (uint32_t*)(data + pos);
        pos += sizeof(uint32_t) + *arglen + 1;
        lua_pushnil(L);
    }
    else
    {
        pos = master_ipc_thread_console_message_push_string(ipcThread, L, data, pos, length);
    }
    
    // cmd
    if (argCount != 0)
    {
        pos = master_ipc_thread_console_message_push_string(ipcThread, L, data, pos, length);
        argCount--;
    }
    else
    {
        lua_pushnil(L);
    }
    
    // args
    if (argCount > 0)
        pos = master_ipc_thread_console_message_table(ipcThread, L, data, pos, length, argCount);
    else
        lua_newtable(L);
    
    if (optCount > 0)
        pos = master_ipc_thread_console_message_table(ipcThread, L, data, pos, length, optCount);
    else
        lua_newtable(L);
    
    if (lua_pcall(L, 7, 1, 0))
    {
        log_format(B(ipcThread), LogError, "[master_ipc_thread_handle_console_message] %s", lua_tostring(L, -1));
    }
    else
    {
        ret = lua_toboolean(L, -1);
    }
    
    lua_settop(L, 0);

    return ret;
}

static int master_ipc_thread_wrap_console_message(R(MasterIpcThread*) ipcThread, R(Master*) M, R(lua_State*) L, R(IpcPacket*) packet)
{
    ChildProcess proc;
    uint32_t length     = ipc_packet_length(packet);
    byte* data          = ipc_packet_data(packet);
    uint32_t pos        = sizeof(uint32_t) * 3; // argCount + optCount + char count for IPC path
    const char* path    = (const char*)(data + pos);
    int ret             = false;
    
    // Make sure there is something where we expect the ipc buffer path...
    if (length > pos && *path != 0)
    {
        shm_viewer_init(&proc.shmViewer);
        shm_viewer_open(B(M), &proc.shmViewer, path, sizeof(IpcBuffer));
        proc.ipc = shm_viewer_memory_type(&proc.shmViewer, IpcBuffer);
        
        ret = master_ipc_thread_handle_console_message(ipcThread, M, L, packet, &proc, 1);
        
        shm_viewer_close(&proc.shmViewer);
    }
    
    return ret;
}

static int master_ipc_thread_handle_packet(R(MasterIpcThread*) ipcThread, R(Master*) M, R(lua_State*) L, R(IpcPacket*) packet)
{
    ServerOp opcode     = ipc_packet_opcode(packet);
    int sourceId        = ipc_packet_source_id(packet);
    ChildProcess* proc;
    
    if (sourceId == EQP_SOURCE_ID_CONSOLE)
        return master_ipc_thread_wrap_console_message(ipcThread, M, L, packet);
    
    proc = master_get_child_process(M, sourceId);
    
    if (proc == NULL)
    {
        log_format(B(ipcThread), LogError, "Received IPC request from unknown or inactive process, sourceId: %i", sourceId);
        return false;
    }
    
    if (proc->ipc == NULL)
    {
        log_format(B(ipcThread), LogError, "Received IPC request from process with NULL IpcBuffer, sourceId: %i, pid: %u", sourceId, proc->pid);
        return false;
    }
    
    proc_update_last_activity_time(proc);
    
    switch (opcode)
    {
    case ServerOp_KeepAlive:
        break;
    
    case ServerOp_ConsoleMessage:
        if (master_ipc_thread_handle_console_message(ipcThread, M, L, packet, proc, 0))
            return true;
        break;
    
    default:
        log_format(B(ipcThread), LogError, "Received unexpected ServerOp: %u", opcode);
        break;
    }
    
    return false;
}

void master_ipc_thread_main_loop(R(Thread*) thread)
{
    R(MasterIpcThread*) ipcThread   = (MasterIpcThread*)thread;
    R(Master*) M                    = master_ipc_thread_master(ipcThread);
    R(IpcBuffer*) ipc               = master_ipc_thread_ipc_buffer(ipcThread);
    lua_State* L                    = NULL;
    
    L = luaL_newstate();
    
    if (L == NULL)
        exception_throw_message(B(ipcThread), ErrorLua, "Failed to load luajit", 0);
    
    luaL_openlibs(L);
    
    if (luaL_loadfile(L, EQP_MASTER_IPC_SCRIPT) || lua_pcall(L, 0, 0, 0))
    {
        ExceptionScope exScope;
        
        switch (exception_try(B(ipcThread), &exScope))
        {
        case Try:
            exception_throw_format(B(ipcThread), ErrorLua, "Error running " EQP_MASTER_IPC_SCRIPT ":\n%s", lua_tostring(L, -1));
            break;
        
        case Finally:
            lua_close(L);
            break;
        
        default: break;
        }
        
        exception_end_try_with_finally(B(ipcThread));
    }
    
    for (;;)
    {
        // Block until we receive an IPC packet
        ipc_buffer_wait(B(ipcThread), ipc);
        
        for (;;)
        {
            IpcPacket packet;
            int done;
            
            if (!ipc_buffer_read(B(ipcThread), ipc, &packet))
                break;
            
            done = master_ipc_thread_handle_packet(ipcThread, M, L, &packet);
            
            ipc_packet_deinit(&packet);
            
            if (done)
                goto finish;
        }
    }
    
finish:
    atomic_mutex_unlock(&ipcThread->mutexShutdown);
    lua_close(L);
}
