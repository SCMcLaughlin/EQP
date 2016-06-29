
#include "atomic_mutex.h"

void atomic_mutex_init(AtomicMutex* mutex)
{
    atomic_flag_clear(mutex);
}

void atomic_mutex_lock(AtomicMutex* mutex)
{
    for (;;)
    {
        if (atomic_mutex_try_lock(mutex))
            return;
    }
}

void atomic_mutex_unlock(AtomicMutex* mutex)
{
    atomic_flag_clear(mutex);
}

int atomic_mutex_try_lock(AtomicMutex* mutex)
{
    return (atomic_flag_test_and_set(mutex) == false);
}
