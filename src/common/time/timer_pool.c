
#include "timer_pool.h"
#include "eqp_basic.h"

#define DEFAULT_CAPACITY 32

void timer_pool_init(R(Basic*) basic, R(TimerPool*) pool)
{
    pool->basic     = basic;
    pool->capacity  = DEFAULT_CAPACITY;
    pool->count     = 0;
    
    pool->triggerTimes  = eqp_alloc_type_array(basic, DEFAULT_CAPACITY, uint64_t);
    pool->timerObjects  = eqp_alloc_type_array(basic, DEFAULT_CAPACITY, Timer*);
    
    pool->triggered     = array_create_type(basic, uint32_t);
}

void timer_pool_deinit(R(TimerPool*) pool)
{
    if (pool->triggerTimes)
    {
        free(pool->triggerTimes);
        pool->triggerTimes = NULL;
    }
    
    if (pool->timerObjects)
    {
        free(pool->timerObjects);
        pool->timerObjects = NULL;
    }
    
    if (pool->triggered)
    {
        array_destroy(pool->triggered);
        pool->triggered = NULL;
    }
}

static void timer_pool_swap_and_pop(R(TimerPool*) pool, uint32_t index)
{
    uint32_t back = --pool->count;
    
    if (back != index)
    {
        Timer* timer                = pool->timerObjects[back];
        pool->timerObjects[index]   = timer;
        pool->triggerTimes[index]   = pool->triggerTimes[back];
        timer->poolIndex            = index;
    }
}

static void timer_pool_trigger(R(TimerPool*) pool, uint32_t index)
{
    Timer* timer = pool->timerObjects[index];
    
    pool->triggerTimes[index] += timer_period_milliseconds(timer);
    
    timer_execute_callback(timer);
}

void timer_pool_execute_callbacks(R(TimerPool*) pool)
{
    uint32_t i              = 0;
    uint32_t n              = pool->count;
    uint64_t* triggerTimes  = pool->triggerTimes;
    uint64_t curTime        = clock_milliseconds();
    Basic* basic            = pool->basic;
    
    // Guarantee: nothing in this loop will cause triggerTimes to relocate
    while (i < n)
    {
        uint64_t triggerTime = triggerTimes[i];
        
        // Is the 'dead' bit set?
        if (bit_get64(triggerTime, 63))
        {
            timer_pool_swap_and_pop(pool, i);
            n--;
            continue;
        }
        
        // Don't count the 'dead' bit as part of the trigger time
        if (curTime >= (triggerTime & bit_mask64(63)))
            array_push_back(basic, &pool->triggered, &i);
        
        i++;
    }
    
    n = array_count(pool->triggered);
    
    if (n > 0)
    {
        uint32_t* triggered = array_data_type(pool->triggered, uint32_t);
        
        for (i = 0; i < n; i++)
        {
            timer_pool_trigger(pool, triggered[i]);
        }
        
        array_clear(pool->triggered);
    }
}

static void timer_pool_realloc(R(TimerPool*) pool)
{
    uint32_t c = pool->capacity * 2;
    
    pool->triggerTimes = eqp_realloc_type_array(pool->basic, pool->triggerTimes, c, uint64_t);
    pool->timerObjects = eqp_realloc_type_array(pool->basic, pool->timerObjects, c, Timer*);
    
    pool->capacity = c;
}

void timer_pool_internal_start_timer(R(TimerPool*) pool, R(Timer*) timer)
{
    uint64_t curTime    = clock_milliseconds();
    uint32_t index      = pool->count++;
    
    if (index >= pool->capacity)
        timer_pool_realloc(pool);
    
    pool->triggerTimes[index] = curTime + timer_period_milliseconds(timer);
    pool->timerObjects[index] = timer;
    
    timer->poolIndex = index;
}

void timer_pool_internal_restart_timer(R(TimerPool*) pool, R(Timer*) timer)
{
    pool->triggerTimes[timer->poolIndex] = clock_milliseconds() + timer_period_milliseconds(timer);
}

void timer_pool_internal_delay_timer(R(TimerPool*) pool, R(Timer*) timer, uint32_t milliseconds)
{
    pool->triggerTimes[timer->poolIndex] += milliseconds;
}

void timer_pool_internal_force_timer_trigger_next_cycle(R(TimerPool*) pool, R(Timer*) timer)
{
    pool->triggerTimes[timer->poolIndex] = clock_milliseconds();
}

void timer_pool_internal_mark_timer_as_dead(R(TimerPool*) pool, uint32_t index)
{
    // The 63rd bit (counting from 0) is the 'dead' flag bit
    pool->triggerTimes[index] |= 1ULL << 63;
}

#undef DEFAULT_CAPACITY
