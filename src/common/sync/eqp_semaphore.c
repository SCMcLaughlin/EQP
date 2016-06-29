
#include "eqp_semaphore.h"
#include "exception.h"

void semaphore_init(Basic* basic, Semaphore* sem)
{
#ifdef EQP_WINDOWS
    *sem = CreateSemaphore(NULL, 0, 2147483647, NULL);
    
    if (!m_semaphore)
#else
    if (sem_init(sem, 1, 0))
#endif
        exception_throw(basic, ErrorSyncPrimitive);
}

void semaphore_deinit(Basic* basic, Semaphore* sem)
{
#ifdef EQP_WINDOWS
    if (CloseHandle(*sem))
#else
    if (sem_destroy(sem))
#endif
        exception_throw(basic, ErrorSyncPrimitive);
}

void semaphore_wait(Basic* basic, Semaphore* sem)
{
#ifdef EQP_WINDOWS
    if (WaitForSingleObject(*sem, INFINITE))
#else
    if (sem_wait(sem))
#endif
        exception_throw(basic, ErrorSyncPrimitive);
}

int semaphore_try_wait(Basic* basic, Semaphore* sem)
{
#ifdef EQP_WINDOWS
    if (WaitForSingleObject(*sem, 0))
        return false;
#else
    if (sem_trywait(sem))
    {
        int err = errno;
        
        if (err == EAGAIN)
            return false;
        
        exception_throw(basic, ErrorSyncPrimitive);
    }
#endif
    
    return true;
}

void semaphore_trigger(Basic* basic, Semaphore* sem)
{
#ifdef EQP_WINDOWS
    if (ReleaseSemaphore(*sem, 1, NULL))
#else
    if (sem_post(sem))
#endif
        exception_throw(basic, ErrorSyncPrimitive);
}
