
#ifndef EQP_SEMAPHORE_H
#define EQP_SEMAPHORE_H

#include "define.h"
#include "exception.h"

#ifdef EQP_LINUX
# include <semaphore.h>
#endif

#ifdef EQP_WINDOWS
typedef HANDLE Semaphore;
#else
typedef sem_t Semaphore;
#endif

STRUCT_DECLARE(Basic);

void    semaphore_init(R(Basic*) basic, R(Semaphore*) sem);
void    semaphore_deinit(R(Basic*) basic, R(Semaphore*) sem);
void    semaphore_wait(R(Basic*) basic, R(Semaphore*) sem);
int     semaphore_try_wait(R(Basic*) basic, R(Semaphore*) sem);
void    semaphore_trigger(R(Basic*) basic, R(Semaphore*) sem);

#endif//EQP_SEMAPHORE_H
