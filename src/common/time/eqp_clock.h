
#ifndef EQP_CLOCK_H
#define EQP_CLOCK_H

#include "define.h"
#include <time.h>

#ifdef EQP_WINDOWS
# include <windows.h>
#endif

STRUCT_DEFINE(PerfTimer)
{
    uint64_t microseconds;
};

EQP_API uint64_t    clock_milliseconds();
EQP_API uint64_t    clock_microseconds();
EQP_API uint64_t    clock_unix_seconds();
void                clock_sleep_milliseconds(uint32_t ms);

void                perf_init(PerfTimer* timer);
uint64_t            perf_microseconds(PerfTimer* timer);

#endif//EQP_CLOCK_H
