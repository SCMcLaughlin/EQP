
#ifndef EQP_ZONE_H
#define EQP_ZONE_H

#include "define.h"
#include "eqp_array.h"
#include "eqp_string.h"
#include "eqp_alloc.h"
#include "lua_object.h"
#include "mob.h"
#include "zone_id.h"

#define EQP_ZONE_MAX_INSTANCE_ID 999

STRUCT_DECLARE(ZC);

STRUCT_DEFINE(Zone)
{
    // LuaObject must be the first member of this struct
    LuaObject   luaObj;
    int         sourceId;
    uint16_t    zoneId;
    uint16_t    instanceId;
    
    Array*      mobsByEntityId;
    Array*      mobsByPosition;
    Array*      clientList;
    Array*      npcList;
};

STRUCT_DEFINE(ZoneBySourceId)
{
    int     sourceId;
    Zone*   zone;
};

Zone*   zone_create(R(ZC*) zc, int sourceId, int zoneId, int instId);
void    zone_destroy(R(ZC*) zc, R(Zone*) zone);

/* LuaJIT API */
EQP_API int         zone_get_source_id(R(Zone*) zone);
EQP_API uint16_t    zone_get_zone_id(R(Zone*) zone);
EQP_API uint16_t    zone_get_instance_id(R(Zone*) zone);
EQP_API const char* zone_get_short_name(R(Zone*) zone);
EQP_API const char* zone_get_long_name(R(Zone*) zone);

#endif//EQP_ZONE_H
