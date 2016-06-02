
#ifndef EQP_THREAD_H
#define EQP_THREAD_H

#include "define.h"
#include "eqp_basic.h"
#include "atomic_mutex.h"
#include "eqp_semaphore.h"
#include "eqp_clock.h"

#ifdef EQP_LINUX
# include <pthread.h>
#endif

STRUCT_DECLARE(Thread);

#define T(obj) ((Thread*)(obj))

typedef void(*ThreadProc)(R(Thread*));

STRUCT_DEFINE(Thread)
{
    // Basic must be the first member of this struct
    Basic           basicState;
    ThreadProc      func;
    AtomicMutex     mutexLifetime;
    AtomicMutex     mutexShouldContinue;
    Semaphore       semaphore;
};

void    thread_start_and_detach(R(Basic*) basic, R(Thread*) thread, ThreadProc func);
void    thread_send_stop_signal(R(Basic*) basic, R(Thread*) thread);
void    thread_wait_until_stopped(R(Thread*) thread);

#define thread_wait(thread) semaphore_wait(B(thread), &(thread)->semaphore)
#define thread_trigger(basic, thread) semaphore_trigger(basic, &(thread)->semaphore)
#define thread_should_stop(thread) (atomic_mutex_try_lock(&(thread)->mutexShouldContinue))

#endif//EQP_THREAD_H
