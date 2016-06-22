
#ifndef EQP_ZONE_H
#define EQP_ZONE_H

#include "define.h"
#include "eqp_array.h"
#include "eqp_string.h"

STRUCT_DEFINE(Zone)
{
    int     sourceId;
    
    Array*  clientList;
    Array*  npcList;
    
    String* longName;
    String* shortName;
};

STRUCT_DEFINE(ZoneBySourceId)
{
    int     sourceId;
    Zone*   zone;
};

#endif//EQP_ZONE_H
