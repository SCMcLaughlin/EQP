
#ifndef EQP_TIMER_POOL_H
#define EQP_TIMER_POOL_H

#include "define.h"
#include "timer.h"
#include "eqp_array.h"
#include "eqp_clock.h"
#include "bit.h"
#include "atomic_mutex.h"
#include "eqp_alloc.h"

STRUCT_DECLARE(Basic);

STRUCT_DEFINE(TimerPool)
{
    Basic*      basic;
    uint32_t    capacity;
    uint32_t    count;
    uint64_t*   triggerTimes;
    Timer**     timerObjects;
    Array*      triggered;
};

void    timer_pool_init(Basic* basic, TimerPool* pool);
void    timer_pool_deinit(TimerPool* pool);

void    timer_pool_execute_callbacks(TimerPool* pool);

// The below functions are for internal use (by Timer functions) only

void    timer_pool_internal_start_timer(TimerPool* pool, Timer* timer);
void    timer_pool_internal_restart_timer(TimerPool* pool, Timer* timer);
void    timer_pool_internal_delay_timer(TimerPool* pool, Timer* timer, uint32_t milliseconds);
void    timer_pool_internal_force_timer_trigger_next_cycle(TimerPool* pool, Timer* timer);
void    timer_pool_internal_mark_timer_as_dead(TimerPool* pool, uint32_t index);

#endif//EQP_TIMER_POOL_H
