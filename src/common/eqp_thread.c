
#include "eqp_thread.h"
#include "eqp_string.h"
#include "eqp_log.h"

static void thread_init(Thread* thread, Basic* basic, ThreadProc func)
{
    basic_copy(B(thread), basic);
    thread->func = func;
    atomic_mutex_init(&thread->mutexLifetime);
    atomic_mutex_init(&thread->mutexShouldContinue);
    semaphore_init(basic, &thread->semaphore);
}

#ifdef EQP_LINUX
static void* thread_proc_wrapper(void* thread)
#else
static void thread_proc_wrapper(void* thread)
#endif
{
    ExceptionScope exScope;
    int ex;
    
    switch ((ex = exception_try(B(thread), &exScope)))
    {
    case Try:
        T(thread)->func(T(thread));
        break;
    
    case Finally:
        atomic_mutex_unlock(&T(thread)->mutexLifetime);
        break;
    
    default:
        if (basic_source_id(B(thread)) != EQP_SOURCE_ID_LOG_WRITER)
            log_format(B(thread), LogFatal, "Unhandled exception type %i in thread: %s\n", ex, string_data(exception_get_message(B(thread))));
        exception_handled(B(thread));
        break;
    }
    
    exception_end_try_with_finally(B(thread));
    basic_deinit(B(thread));
    
#ifdef EQP_LINUX
    return NULL;
#endif
}

void thread_start_and_detach(Basic* basic, Thread* thread, ThreadProc func)
{
#ifdef EQP_LINUX
    pthread_t pthread;
#endif
    
    basic_preinit(B(thread));
    thread_init(thread, basic, func);
    atomic_mutex_lock(&thread->mutexLifetime);
    atomic_mutex_lock(&thread->mutexShouldContinue);
    
#ifdef EQP_LINUX
    pthread_create(&pthread, NULL, thread_proc_wrapper, thread);
    pthread_detach(pthread);
#else
    
#endif
}

void thread_send_stop_signal(Basic* basic, Thread* thread)
{
    atomic_mutex_unlock(&thread->mutexShouldContinue);
    thread_trigger(basic, thread);
}

void thread_wait_until_stopped(Thread* thread)
{
    for (;;)
    {
        if (atomic_mutex_try_lock(&thread->mutexLifetime))
            break;
        
        clock_sleep_milliseconds(10);
    }
    
    atomic_mutex_unlock(&thread->mutexLifetime);
}
