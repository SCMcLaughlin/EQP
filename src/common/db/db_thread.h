
#ifndef EQP_DB_THREAD_H
#define EQP_DB_THREAD_H

#include "define.h"
#include "eqp_thread.h"
#include "eqp_array.h"
#include "query.h"
#include "transaction.h"

#define EQP_DB_CALC_LENGTH (-1)

STRUCT_DEFINE(DbThread)
{
    Thread      thread;
    AtomicMutex mutexInQueue;
    Array*      inQueue;
    Array*      inTransactionQueue;
    AtomicMutex mutexOutQueue;
    Array*      outQueue;
    Array*      executeQueue;
};

void    db_thread_init(Basic* basic, DbThread* dbThread);
void    db_thread_deinit(DbThread* dbThread);
void    db_thread_main_loop(Thread* thread);

#define db_thread_start(basic, dbt) thread_start_and_detach((basic), T(dbt), db_thread_main_loop)

void    db_thread_schedule_query(Basic* basic, DbThread* dbThread, Query* query);
void    db_thread_schedule_transaction(Basic* basic, DbThread* dbThread, Transaction* trans);
void    db_thread_execute_query_callbacks(DbThread* dbThread);

#endif//EQP_DB_THREAD_H
