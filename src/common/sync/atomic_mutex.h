
#ifndef EQP_ATOMIC_MUTEX_H
#define EQP_ATOMIC_MUTEX_H

#include "define.h"
#include <stdatomic.h>

typedef atomic_flag AtomicMutex;

void    atomic_mutex_init(AtomicMutex* mutex);
void    atomic_mutex_lock(AtomicMutex* mutex);
void    atomic_mutex_unlock(AtomicMutex* mutex);
int     atomic_mutex_try_lock(AtomicMutex* mutex);

#endif//EQP_ATOMIC_MUTEX_H
