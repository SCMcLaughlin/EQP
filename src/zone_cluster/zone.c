
#include "zone.h"
#include "zone_cluster.h"

Zone* zone_create(R(ZC*) zc, int sourceId, int zoneId, int instId)
{
    Zone* zone = eqp_alloc_type(B(zc), Zone);
    uint32_t i;
    
    memset(zone, 0, sizeof(Zone));
    
    zone->sourceId      = sourceId;
    zone->zoneId        = zoneId;
    zone->instanceId    = instId;
    
    zone->mobsByEntityId    = array_create_type(B(zc), MobByEntityId);
    zone->mobsByPosition    = array_create_type(B(zc), MobByPosition);
    
    for (i = 0; i < 4; i++)
    {
        zone->fogStats[i].minClippingDistance = 10.0f;
        zone->fogStats[i].maxClippingDistance = 20000.0f;
    }
    
    zone->zoneType              = 0xff;
    zone->gravity               = 0.4f;
    zone->minZ                  = -32000.0f;
    zone->minClippingDistance   = 1000.0f;
    zone->maxClippingDistance   = 20000.0f;
    
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
    
    zc_lua_destroy_object(zc, &zone->luaObj);
    
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
