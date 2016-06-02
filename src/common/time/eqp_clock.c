
#include "eqp_clock.h"

#ifdef EQP_WINDOWS
static LARGE_INTEGER sFrequency;

static LARGE_INTEGER windowsPerfTime()
{
    static BOOL sUseQpc = QueryPerformanceFrequency(&sFrequency);
    
    LARGE_INTEGER now;
    
    HANDLE thread       = GetCurrentThread();
    DWORD_PTR oldMask   = SetThreadAffinityMask(thread, 0);
    
    QueryPerformanceCounter(&now);
    SetThreadAffinityMask(thread, oldMask);
    
    return now;
}
#endif

uint64_t clock_milliseconds()
{
#ifdef EQP_WINDOWS
    LARGE_INTEGER li = windowsPerfTime();
    return (uint64_t)((1000LL * li.QuadPart) / sFrequency.QuadPart);
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000 + t.tv_nsec / 1000000;
#endif
}

uint64_t clock_microseconds()
{
#ifdef EQP_WINDOWS
    LARGE_INTEGER li = windowsPerfTime();
    return (uint64_t)((1000000LL * li.QuadPart) / sFrequency.QuadPart);
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000 + t.tv_nsec / 1000;
#endif
}

uint64_t clock_unix_seconds()
{
#ifdef EQP_WINDOWS
    return _time64(NULL);
#else
    return time(NULL);
#endif
}

void clock_sleep_milliseconds(uint32_t ms)
{
#ifdef EQP_WINDOWS
    Sleep(ms);
#else
    struct timespec t;
    t.tv_sec    = ms / 1000;
    t.tv_nsec   = ms * 1000000;
    nanosleep(&t, NULL);
#endif
}

void perf_init(PerfTimer* timer)
{
    timer->microseconds = clock_microseconds();
}

uint64_t perf_microseconds(PerfTimer* timer)
{
    return clock_microseconds() - timer->microseconds;
}
