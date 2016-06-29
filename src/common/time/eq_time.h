
#ifndef EQP_EQ_TIME_H
#define EQP_EQ_TIME_H

#include "define.h"
#include "eqp_clock.h"

STRUCT_DEFINE(EQ_Time)
{
    uint8_t     hour;
    uint8_t     minute;
    uint8_t     day;
    uint8_t     month;
    uint16_t    year;
};

void    eq_time_calc(EQ_Time* eqTime, uint64_t baseUnixSeconds);

#endif//EQP_EQ_TIME_H
