
#ifndef EQP_DB_THREAD_H
#define EQP_DB_THREAD_H

#include "define.h"
#include "eqp_thread.h"
#include "eqp_array.h"
#include "query.h"

#define EQP_DB_CALC_LENGTH (-1)

STRUCT_DEFINE(DbThread)
{
    Thread      thread;
    AtomicMutex mutexInQueue;
    Array*      inQueue;
    AtomicMutex mutexOutQueue;
    Array*      outQueue;
    Array*      executeQueue;
};

void    db_thread_init(R(Basic*) basic, R(DbThread*) dbThread);
void    db_thread_deinit(R(DbThread*) dbThread);
void    db_thread_main_loop(R(Thread*) thread);

#define db_thread_start(basic, dbt) thread_start_and_detach((basic), T(dbt), db_thread_main_loop)

void    db_thread_schedule_query(R(Basic*) basic, R(DbThread*) dbThread, R(Query*) query);
void    db_thread_execute_query_callbacks(R(DbThread*) dbThread);

#endif//EQP_DB_THREAD_H
