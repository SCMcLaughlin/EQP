
#ifndef EQP_TIMER_H
#define EQP_TIMER_H

#include "define.h"

STRUCT_DECLARE(Timer);
STRUCT_DECLARE(TimerPool);

typedef void(*TimerCallback)(R(Timer*) timer);

STRUCT_DEFINE(Timer)
{
    TimerPool*      pool;
    uint32_t        poolIndex;
    uint32_t        periodMilliseconds;
    TimerCallback   callback;
    
    union
    {
        void*   userdata;
        int     luaCallback;
    };
};

void    timer_init(R(Timer*) timer, R(TimerPool*) pool, uint32_t periodMilliseconds, TimerCallback callback, void* userdata, int start);
void    timer_deinit(R(Timer*) timer);

#define timer_period_milliseconds(timer) ((timer)->periodMilliseconds)
#define timer_userdata(timer) ((timer)->userdata)

void    timer_stop(R(Timer*) timer);
#define timer_start(timer) timer_restart((timer))
void    timer_restart(R(Timer*) timer);

// Delaying an inactive/stopped timer has no effect
void    timer_delay(R(Timer*) timer, uint32_t milliseconds);

// Forces the timer to trigger on the next cycle, as if its trigger time was reached.
// If the timer is currently inactive/stopped, it is automatically restarted first.
void    timer_force_trigger_on_next_cycle(R(Timer*) timer);

// Invokes the timer's callback immediately, independent of the timing of its normal trigger cycle.
void    timer_execute_callback(R(Timer*) timer);

#endif//_EQP_TIMER_H
