
#ifndef EQP_MASTER_IPC_H
#define EQP_MASTER_IPC_H

#include "define.h"
#include "eqp_thread.h"
#include "ipc_buffer.h"
#include "atomic_mutex.h"
#include "server_op.h"
#include "source_id.h"
#include "database.h"
#include "db_thread.h"
#include "lua_sys.h"
#include "console_interface.h"
#include "client_manager.h"

#define EQP_MASTER_IPC_SCRIPT "scripts/master/master_ipc.lua"

STRUCT_DECLARE(Master);

STRUCT_DEFINE(MasterIpcThread)
{
    Thread      thread;
    Master*     master;
    IpcBuffer*  ipcMaster;
    AtomicMutex mutexShutdown;
    
    lua_State*  L;
    Database*   db;
    ClientMgr   clientMgr;
};

void    master_ipc_thread_init(R(Master*) M, R(MasterIpcThread*) ipcThread);
#define master_ipc_thread_master(thread) ((thread)->master)
#define master_ipc_thread_ipc_buffer(thread) ((thread)->ipcMaster)
void    master_ipc_thread_main_loop(R(Thread*) thread);
#define master_ipc_thread_start(basic, thread) thread_start_and_detach((basic), T(thread), master_ipc_thread_main_loop)
#define master_ipc_thread_shutdown_received(thread) atomic_mutex_try_lock(&(thread)->mutexShutdown)

#define master_ipc_thread_lua(thread) ((thread)->L)
#define master_ipc_thread_db(thread) ((thread)->db)
#define master_ipc_thread_lock_init(thread) client_mgr_lock_init(&(thread)->clientMgr)
#define master_ipc_thread_lock(thread) client_mgr_lock(&(thread)->clientMgr)
#define master_ipc_thread_unlock(thread) client_mgr_unlock(&(thread)->clientMgr)

#endif//EQP_MASTER_IPC_H
