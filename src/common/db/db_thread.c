
#include "db_thread.h"
#include "database.h"
#include "eqp_string.h"

void db_thread_init(Basic* basic, DbThread* dbThread)
{
    atomic_mutex_init(&dbThread->mutexInQueue);
    atomic_mutex_init(&dbThread->mutexOutQueue);
    
    dbThread->inQueue               = array_create_type(basic, Query);
    dbThread->inTransactionQueue    = array_create_type(basic, Transaction);
    dbThread->outQueue              = array_create_type(basic, Query);
    dbThread->executeQueue          = array_create_type(basic, Query);
}

void db_thread_deinit(DbThread* dbThread)
{
    if (dbThread->inQueue)
    {
        array_destroy(dbThread->inQueue);
        dbThread->inQueue = NULL;
    }
    
    if (dbThread->inTransactionQueue)
    {
        array_destroy(dbThread->inTransactionQueue);
        dbThread->inTransactionQueue = NULL;
    }
    
    if (dbThread->outQueue)
    {
        array_destroy(dbThread->outQueue);
        dbThread->outQueue = NULL;
    }
    
    if (dbThread->executeQueue)
    {
        array_destroy(dbThread->executeQueue);
        dbThread->executeQueue = NULL;
    }
}

static void db_thread_run_transactions(DbThread* dbThread)
{
    atomic_mutex_lock(&dbThread->mutexInQueue);
            
    if (!array_empty(dbThread->inTransactionQueue))
    {
        Transaction trans;
        Query query;
        TransactionCallback callback;
        
        array_back_copy(dbThread->inTransactionQueue, &trans);
        array_pop_back(dbThread->inTransactionQueue);
        
        atomic_mutex_unlock(&dbThread->mutexInQueue);
        
        query_init(&query);
        db_prepare_literal(transaction_db(&trans), &query, "BEGIN", NULL);
        query_execute_synchronus(&query);
        query_deinit(&query);
        
        callback = transaction_callback(&trans);
        if (callback)
            callback(&trans);
        
        query_init(&query);
        query_set_userdata(&query, transaction_userdata(&trans));
        db_prepare_literal(transaction_db(&trans), &query, "COMMIT", transaction_query_callback(&trans));
        query_execute_synchronus(&query);
        
        if (transaction_query_callback(&trans))
        {
            atomic_mutex_lock(&dbThread->mutexOutQueue);
            array_push_back(B(dbThread), &dbThread->outQueue, &query);
            atomic_mutex_unlock(&dbThread->mutexOutQueue);
        }
        else
        {
            query_deinit(&query);
        }
        
        atomic_mutex_lock(&dbThread->mutexInQueue);
    }
    
    atomic_mutex_unlock(&dbThread->mutexInQueue);
}

static void db_thread_read_newly_scheduled(DbThread* dbThread)
{
    atomic_mutex_lock(&dbThread->mutexInQueue);
            
    if (!array_empty(dbThread->inQueue))
    {
        uint32_t count  = array_count(dbThread->inQueue);
        Query* queries  = array_data_type(dbThread->inQueue, Query);
        uint32_t i;
        
        for (i = 0; i < count; i++)
        {
            array_push_back(B(dbThread), &dbThread->executeQueue, &queries[i]);
        }
        
        array_clear(dbThread->inQueue);
    }
    
    atomic_mutex_unlock(&dbThread->mutexInQueue);
}

static void db_thread_execute_queries(DbThread* dbThread)
{
    Query* volatile queries = array_data_type(dbThread->executeQueue, Query);
    uint32_t volatile n     = array_count(dbThread->executeQueue);
    uint32_t volatile i     = 0;
    
    while (i < n)
    {
        ExceptionScope exScope;
        Query* volatile query = &queries[i];
        
        exception_begin_try(B(dbThread), &exScope);
        
        switch (exception_try(B(dbThread)))
        {
        case Try:
            if (query_execute_background(query))
            {
                if (query_has_callback(query))
                {
                    atomic_mutex_lock(&dbThread->mutexOutQueue);
                    array_push_back(B(dbThread), &dbThread->outQueue, query);
                    atomic_mutex_unlock(&dbThread->mutexOutQueue);
                }
                else
                {
                    query_deinit(query);
                }
                
                array_swap_and_pop(dbThread->executeQueue, i);
                n--;
                goto next;
            }
            
            i++;
            break;
        
        default:
            // If the query throws an error while executing, discard it and move on
            log_format(B(dbThread), LogSql, "[db_thread_execute_queries] Error executing query %u: %s\n", query_get_id(query), string_data(exception_get_message(B(dbThread))));
            exception_handled(B(dbThread));
            array_swap_and_pop(dbThread->executeQueue, i);
            n--;
            break;
        }
        
    next:
        exception_end_try(B(dbThread));
    }
}

void db_thread_main_loop(Thread* thread)
{
    DbThread* dbThread = (DbThread*)thread;
    
    for (;;)
    {
        thread_wait(T(dbThread));
        
        for (;;)
        {
            db_thread_run_transactions(dbThread);
            db_thread_read_newly_scheduled(dbThread);
            db_thread_execute_queries(dbThread);

            if (array_empty(dbThread->executeQueue))
                break;
            
            clock_sleep_milliseconds(2);
        }
        
        if (thread_should_stop(T(dbThread)))
            return;
    }
}

void db_thread_schedule_query(Basic* basic, DbThread* dbThread, Query* query)
{
    AtomicMutex* mutex = &dbThread->mutexInQueue;
    
    atomic_mutex_lock(mutex);
    array_push_back(basic, &dbThread->inQueue, query);
    atomic_mutex_unlock(mutex);
    
    // Reset the original copy of the query
    query_init(query);
    // Trigger the thread loop in case it's not already running
    thread_trigger(basic, T(dbThread));
}

void db_thread_schedule_transaction(Basic* basic, DbThread* dbThread, Transaction* trans)
{
    AtomicMutex* mutex = &dbThread->mutexInQueue;
    
    atomic_mutex_lock(mutex);
    array_push_back(basic, &dbThread->inTransactionQueue, trans);
    atomic_mutex_unlock(mutex);
    
    // Trigger the thread loop in case it's not already running
    thread_trigger(basic, T(dbThread));
}

void db_thread_execute_query_callbacks(DbThread* dbThread)
{
    AtomicMutex* mutex = &dbThread->mutexOutQueue;
    
    atomic_mutex_lock(mutex);
    
    while (!array_empty(dbThread->outQueue))
    {
        Query query;
        array_back_copy(dbThread->outQueue, &query);
        array_pop_back(dbThread->outQueue);
        
        // Unlock the mutex while we execute the callback
        atomic_mutex_unlock(mutex);
        
        query_execute_callback(&query);
        query_deinit(&query);
        
        // Re-lock the mutex for the next round
        atomic_mutex_lock(mutex);
    }
    
    atomic_mutex_unlock(mutex);
}
