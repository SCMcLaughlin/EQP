
#ifndef _EQP_ATOMIC_MUTEX_H_
#define _EQP_ATOMIC_MUTEX_H_

#include "define.h"
#include <stdatomic.h>

typedef atomic_flag AtomicMutex;

void    atomic_mutex_init(R(AtomicMutex*) mutex);
void    atomic_mutex_lock(R(AtomicMutex*) mutex);
void    atomic_mutex_unlock(R(AtomicMutex*) mutex);
int     atomic_mutex_try_lock(R(AtomicMutex*) mutex);

#endif//_EQP_ATOMIC_MUTEX_H_
