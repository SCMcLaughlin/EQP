
#include "eq_time.h"

#define SECONDS_PER_EQ_MINUTE   3
#define DAYS_PER_YEAR           365 /* Norrath doesn't follow the Gregorian calendar, ok? */
#define MINUTES_PER_HOUR        60
#define MINUTES_PER_DAY         (MINUTES_PER_HOUR * 24)
#define MINUTES_PER_YEAR        (MINUTES_PER_DAY * DAYS_PER_YEAR)

void eq_time_calc(EQ_Time* eqTime, uint64_t baseUnixSeconds)
{
    uint64_t eq;
    uint32_t day;
    uint32_t i;
    
    static const uint8_t daysInMonth[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    
    // This gets us the EQ time in EQ minutes
    eq = (clock_unix_seconds() - baseUnixSeconds) / SECONDS_PER_EQ_MINUTE;
    
    // Whittling our way down... get the EQ year and factor it out of our EQ minutes
    eqTime->year = eq / MINUTES_PER_YEAR;
    eq %= MINUTES_PER_YEAR;
    
    // Months take some extra work
    day = (eq / MINUTES_PER_DAY) + 1; // There's no zeroth day
    
    for (i = 0; i < 12; i++)
    {
        uint8_t inMonth = daysInMonth[i];
        
        if (day <= inMonth)
        {
            eqTime->month = i + 1; // No zeroth month either
            break;
        }
        
        day -= inMonth;
    }
    
    // Day
    eqTime->day = day;
    eq %= MINUTES_PER_DAY;
    
    // Hour
    eqTime->hour = eq / MINUTES_PER_HOUR;
    eq %= MINUTES_PER_HOUR;
    
    eqTime->minute = eq;
}

#undef SECONDS_PER_EQ_MINUTE
#undef DAYS_PER_YEAR
#undef MINUTES_PER_HOUR
#undef MINUTES_PER_DAY
#undef MINUTES_PER_YEAR
