
#include "timer.h"
#include "timer_pool.h"

#define INVALID_INDEX 0xffffffff

void timer_init(R(Timer*) timer, R(TimerPool*) pool, uint32_t periodMilliseconds, TimerCallback callback, void* userdata, int start)
{
    timer->pool                 = pool;
    timer->poolIndex            = INVALID_INDEX;
    timer->periodMilliseconds   = periodMilliseconds;
    timer->callback             = callback;
    timer->userdata             = userdata;
    
    if (start)
        timer_pool_internal_start_timer(pool, timer);
}

void timer_deinit(R(Timer*) timer)
{
    timer_stop(timer);
}

void timer_stop(R(Timer*) timer)
{
    uint32_t index = timer->poolIndex;
    
    if (index == INVALID_INDEX)
        return;
    
    timer->poolIndex = INVALID_INDEX;
    timer_pool_internal_mark_timer_as_dead(timer->pool, index);
}

void timer_restart(R(Timer*) timer)
{
    TimerPool* pool = timer->pool;
    
    if (timer->poolIndex == INVALID_INDEX)
        timer_pool_internal_start_timer(pool, timer);
    else
        timer_pool_internal_restart_timer(pool, timer);
}

void timer_delay(R(Timer*) timer, uint32_t milliseconds)
{
    if (timer->poolIndex != INVALID_INDEX)
        timer_pool_internal_delay_timer(timer->pool, timer, milliseconds);
}

void timer_force_trigger_on_next_cycle(R(Timer*) timer)
{
    TimerPool* pool = timer->pool;
    
    if (timer->poolIndex == INVALID_INDEX)
        timer_pool_internal_start_timer(pool, timer);
    
    timer_pool_internal_force_timer_trigger_next_cycle(pool, timer);
}

void timer_execute_callback(R(Timer*) timer)
{
    if (timer->callback)
        timer->callback(timer);
}

#undef INVALID_INDEX
