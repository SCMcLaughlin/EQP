
#ifndef EQP_TIMER_H
#define EQP_TIMER_H

#include "define.h"

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(Timer);
STRUCT_DECLARE(TimerPool);

typedef void(*TimerCallback)(Timer* timer);

STRUCT_DEFINE(Timer)
{
    TimerPool*      pool;
    uint32_t        poolIndex;
    uint32_t        periodMilliseconds;
    TimerCallback   callback;
    void*           userdata;
};

void    timer_init(Timer* timer, TimerPool* pool, uint32_t periodMilliseconds, TimerCallback callback, void* userdata, int start);
void    timer_deinit(Timer* timer);
Timer*  eqp_timer_create(Basic* basic, TimerPool* pool, uint32_t periodMilliseconds, TimerCallback callback, void* userdata, int start);
void    timer_destroy(Timer* timer);

#define timer_set_period_milliseconds(timer, ms) ((timer)->periodMilliseconds = (ms))
#define timer_set_callback(timer, cb) ((timer)->callback = (cb))
#define timer_set_userdata(timer, data) ((timer)->userdata = (data))

#define timer_period_milliseconds(timer) ((timer)->periodMilliseconds)
#define timer_userdata(timer) ((timer)->userdata)
#define timer_userdata_type(timer, type) ((type*)((timer)->userdata))

EQP_API void    timer_stop(Timer* timer);
#define         timer_start(timer) timer_restart((timer))
EQP_API void    timer_restart(Timer* timer);

// Delaying an inactive/stopped timer has no effect
EQP_API void    timer_delay(Timer* timer, uint32_t milliseconds);

// Forces the timer to trigger on the next cycle, as if its trigger time was reached.
// If the timer is currently inactive/stopped, it is automatically restarted first.
EQP_API void    timer_force_trigger_on_next_cycle(Timer* timer);

// Invokes the timer's callback immediately, independent of the timing of its normal trigger cycle.
EQP_API void    timer_execute_callback(Timer* timer);

#endif//_EQP_TIMER_H
