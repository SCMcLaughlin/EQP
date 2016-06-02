
#include "atomic_mutex.h"

void atomic_mutex_init(R(AtomicMutex*) mutex)
{
    atomic_flag_clear(mutex);
}

void atomic_mutex_lock(R(AtomicMutex*) mutex)
{
    for (;;)
    {
        if (atomic_mutex_try_lock(mutex))
            return;
    }
}

void atomic_mutex_unlock(R(AtomicMutex*) mutex)
{
    atomic_flag_clear(mutex);
}

int atomic_mutex_try_lock(R(AtomicMutex*) mutex)
{
    return (atomic_flag_test_and_set(mutex) == false);
}
