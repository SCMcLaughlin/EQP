
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

void    semaphore_init(Basic* basic, Semaphore* sem);
void    semaphore_deinit(Basic* basic, Semaphore* sem);
void    semaphore_wait(Basic* basic, Semaphore* sem);
int     semaphore_try_wait(Basic* basic, Semaphore* sem);
void    semaphore_trigger(Basic* basic, Semaphore* sem);

#endif//EQP_SEMAPHORE_H
