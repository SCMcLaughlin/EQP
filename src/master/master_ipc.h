
#ifndef EQP_MASTER_IPC_H
#define EQP_MASTER_IPC_H

#include "define.h"
#include "eqp_thread.h"
#include "ipc_buffer.h"
#include "atomic_mutex.h"
#include "server_op.h"
#include "source_id.h"
#include <lua.h>
#include <lualib.h>
#include <luaconf.h>
#include <lauxlib.h>

#define EQP_MASTER_IPC_SCRIPT "scripts/master/master_ipc.lua"

STRUCT_DECLARE(Master);

STRUCT_DEFINE(MasterIpcThread)
{
    Thread      thread;
    Master*     master;
    IpcBuffer*  ipcMaster;
    AtomicMutex mutexShutdown;
};

void    master_ipc_thread_init(R(Master*) M, R(MasterIpcThread*) ipcThread);
#define master_ipc_thread_master(thread) ((thread)->master)
#define master_ipc_thread_ipc_buffer(thread) ((thread)->ipcMaster)
void    master_ipc_thread_main_loop(R(Thread*) thread);
#define master_ipc_thread_start(basic, thread) thread_start_and_detach((basic), T(thread), master_ipc_thread_main_loop)
#define master_ipc_thread_shutdown_received(thread) atomic_mutex_try_lock(&(thread)->mutexShutdown)

EQP_API void master_ipc_thread_console_reply(R(MasterIpcThread*) ipcThread, R(IpcBuffer*) ipc, R(const char*) src, R(const char*) msg);
EQP_API void master_ipc_thread_console_finish(R(MasterIpcThread*) ipcThread, R(IpcBuffer*) ipc, R(const char*) src);

#endif//EQP_MASTER_IPC_H
