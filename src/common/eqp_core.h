
#ifndef EQP_CORE_H
#define EQP_CORE_H

#include "define.h"
#include "exception.h"
#include "database.h"
#include "db_thread.h"
#include "eqp_basic.h"
#include "eqp_log.h"
#include "eqp_alloc.h"
#include "atomic_mutex.h"
#include "ipc_buffer.h"

/*
    The 'Core' object contains components that are common to (the main threads of)
    Master, CharSelect, Login and ZoneCluster.
*/

STRUCT_DEFINE(Core)
{
    // Basic must be the first member of this struct
    Basic           basicState;
    Database        database;
    DbThread        dbThread;
};

#define C(obj) ((Core*)(obj))

void    core_init(R(Core*) core, int sourceId, R(IpcBuffer*) logIpc);
void    core_deinit(R(Core*) core);

void    core_start_threads(R(Core*) core);
void    core_send_stop_signal_to_threads(R(Core*) core);
void    core_wait_for_threads_to_stop(R(Core*) core);

#define core_is_master(core) ((core)->isMaster)

#define core_db(core) (&(core)->database)
#define core_db_thread(core) (&(core)->dbThread)

#endif//EQP_CORE_H
