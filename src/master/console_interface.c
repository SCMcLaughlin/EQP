
#include "console_interface.h"
#include "master_ipc.h"
#include "eqp_master.h"

STRUCT_DEFINE(ConsoleMessage)
{
    uint32_t    argCount;
    uint32_t    optCount;
    uint32_t    replyToLength; // Does not include null terminator
    char        replyTo[0];
};

static void console_destroy(Console* console)
{
    if (console->sourceId == EQP_SOURCE_ID_CONSOLE)
        proc_deinit(&console->procConsole);
    
    free(console);
}

static void console_lua_push_string(lua_State* L, Aligned* a)
{
    uint32_t len = aligned_read_uint32(a);
    lua_pushlstring(L, (const char*)aligned_current(a), len);
    aligned_advance(a, len + 1); // len does not include the null terminator
}

static void console_lua_push_table(lua_State* L, Aligned* a, uint32_t count)
{
    uint32_t i;
    
    lua_createtable(L, count, 0);
    
    for (i = 1; i <= count; i++)
    {
        lua_pushinteger(L, i);
        console_lua_push_string(L, a);
        lua_settable(L, -3);
    }
}

static int console_push_to_lua(Basic* basic, lua_State* L, Console* console, ConsoleMessage* msg, Aligned* a)
{
    uint32_t argCount   = msg->argCount;
    int ret             = false;
    
    lua_getglobal(L, "handle_console");
    
    if (!lua_isfunction(L, -1))
        exception_throw_literal(basic, ErrorLua, "[console_push_to_lua] Lua function 'handle_console' is undefined");
    
    // Lua function looks like this: handle_console(console, cmd, args, opts)
    // cmd is a string, args and opts are tables of strings
    
    // console
    lua_pushlightuserdata(L, console);
    
    // cmd
    if (argCount != 0)
    {
        console_lua_push_string(L, a);
        argCount--;
    }
    else
    {
        lua_pushnil(L);
    }
    
    // args
    console_lua_push_table(L, a, argCount);
    // opts
    console_lua_push_table(L, a, msg->optCount);
    
    if (lua_sys_call_no_throw(basic, L, 4, 1))
    {
        ret = lua_toboolean(L, -1);
        lua_pop(L, 1);
    }
    
    return ret;
}

static int console_do_receive(MasterIpcThread* ipcThread, Master* M, ChildProcess* proc, int sourceId, IpcPacket* packet, Console* volatile* conPtr)
{
    Aligned read;
    Aligned* a = &read;
    Console* console;
    ConsoleMessage* msg;
    
    aligned_init(B(ipcThread), a, ipc_packet_data(packet), ipc_packet_length(packet));
    
    if (aligned_remaining(a) < sizeof(ConsoleMessage))
        return false;
    
    msg = (ConsoleMessage*)aligned_current(a);
    aligned_advance(a, sizeof(ConsoleMessage));
    
    console             = eqp_alloc_type(B(ipcThread), Console);
    console->M          = M;
    console->ipcThread  = ipcThread;
    console->sourceId   = sourceId;
    
    *conPtr = console;
    
    if (proc)
    {
        console->procPtr                = proc;
        console->replyCharNameLength    = msg->replyToLength;
        snprintf(console->replyCharName, sizeof(console->replyCharName), "%s", (const char*)aligned_current(a));
    }
    else
    {
        proc_init(&console->procConsole);
        proc_open_ipc_buffer(B(ipcThread), &console->procConsole, (const char*)aligned_current(a));
    }
    
    aligned_advance(a, msg->replyToLength + 1);
    
    return console_push_to_lua(B(ipcThread), master_ipc_thread_lua(ipcThread), console, msg, a);
}

int console_receive(MasterIpcThread* ipcThread, Master* M, ChildProcess* proc, int sourceId, IpcPacket* packet)
{
    ExceptionScope exScope;
    Console* volatile console   = NULL;
    int volatile ret            = false;
    int ex;
    
    exception_begin_try(B(ipcThread), &exScope);
    
    switch ((ex = exception_try(B(ipcThread))))
    {
    case Try:
        ret = console_do_receive(ipcThread, M, proc, sourceId, packet, &console);
        break;
    
    default:
        if (console)
            console_destroy(console);
        
        log_format(B(ipcThread), LogError, "[console_receive] Error (%i): %s", ex, string_data(exception_message(B(ipcThread))));
        
        exception_handled(B(ipcThread));
        break;
    }
    
    exception_end_try(B(ipcThread));
    
    return ret;
}

static void console_write(Console* console, const char* msg, uint32_t length, ServerOp opcode)
{
    char buf[EQP_IPC_PACKET_MAX_SIZE];
    ChildProcess* proc;
    Aligned w;
    Basic* basic    = B(console->ipcThread);
    int sourceId    = console->sourceId;
    
    aligned_init(basic, &w, buf, sizeof(buf));
    
    if (sourceId == EQP_SOURCE_ID_CONSOLE)
    {
        proc = &console->procConsole;
    }
    else
    {
        uint32_t len    = console->replyCharNameLength;
        proc            = console->procPtr;
        
        if (len)
            aligned_write_buffer(&w, console->replyCharName, len);
    }
    
    if (msg && length)
        aligned_write_buffer(&w, msg, length);
    
    ipc_buffer_write(basic, proc_ipc(proc), opcode, sourceId, aligned_position(&w), buf);
}

void console_reply(Console* console, const char* msg, uint32_t length)
{
    console_write(console, msg, length, ServerOp_ConsoleMessage);
}

void console_finish(Console* console)
{
    console_write(console, NULL, 0, ServerOp_ConsoleFinish);
    console_destroy(console);
}
