
#include "zone.h"
#include "zone_cluster.h"

Zone* zone_create(R(ZC*) zc, int sourceId, int zoneId, int instId)
{
    Zone* zone = eqp_alloc_type(B(zc), Zone);
    
    memset(zone, 0, sizeof(Zone));
    
    zone->sourceId      = sourceId;
    zone->zoneId        = zoneId;
    zone->instanceId    = instId;
    
    zone->mobsByEntityId    = array_create_type(B(zc), MobByEntityId);
    zone->mobsByPosition    = array_create_type(B(zc), MobByPosition);
    
    zc_lua_create_zone(zc, zone);
    
    return zone;
}

void zone_destroy(R(ZC*) zc, R(Zone*) zone)
{
    if (zone->mobsByEntityId)
    {
        array_destroy(zone->mobsByEntityId);
        zone->mobsByEntityId = NULL;
    }
    
    if (zone->mobsByPosition)
    {
        array_destroy(zone->mobsByPosition);
        zone->mobsByPosition = NULL;
    }
    
    zc_lua_destroy_object(zc, (LuaObject*)zone);
    
    free(zone);
}

/* LuaJIT API */

int zone_get_source_id(R(Zone*) zone)
{
    return zone->sourceId;
}

uint16_t zone_get_zone_id(R(Zone*) zone)
{
    return zone->zoneId;
}

uint16_t zone_get_instance_id(R(Zone*) zone)
{
    return zone->instanceId;
}

const char* zone_get_short_name(R(Zone*) zone)
{
    return zone_short_name_by_id(zone->zoneId);
}

const char* zone_get_long_name(R(Zone*) zone)
{
    return zone_long_name_by_id(zone->zoneId);
}
