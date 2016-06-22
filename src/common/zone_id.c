
#include "zone_id.h"

const char* zone_short_name_by_id(int id)
{
    const char* ret;
    
    switch (id)
    {
    default:
    case 1: ret = "qeynos"; break;
    case 2: ret = "qeynos2"; break;
    case 3: ret = "qrg"; break;
    case 54: ret = "gfaydark"; break;
    }
    
    return ret;
}

const char* zone_long_name_by_id(int id)
{
    const char* ret;
    
    switch (id)
    {
    default:
    case 1: ret = "South Qeynos"; break;
    case 2: ret = "North Qeynos"; break;
    case 3: ret = "The Surefall Glade"; break;
    case 54: ret = "The Greater Faydark"; break;
    }
    
    return ret;
}
