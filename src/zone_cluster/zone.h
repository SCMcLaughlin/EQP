
#ifndef EQP_ZONE_H
#define EQP_ZONE_H

#include "define.h"
#include "eqp_array.h"
#include "eqp_string.h"
#include "eqp_alloc.h"
#include "lua_object.h"
#include "mob.h"
#include "client.h"
#include "zone_id.h"
#include "structs.h"
#include "packet_broadcast.h"

STRUCT_DECLARE(ZC);

STRUCT_DEFINE(Zone)
{
    // LuaObject must be the first member of this struct
    LuaObject   luaObj;
    int         sourceId;
    uint16_t    zoneId;
    uint16_t    instanceId;
    
    Array*      mobsByEntityId; //fixme: use entityId = array index
    Array*      mobsByPosition;
    Array*      clientList;
    Array*      npcList;
    
    Loc         safeSpot;
    FogStats    fogStats[4];
    uint8_t     zoneType;
    uint16_t    skyType;
    float       gravity;
    float       minZ;
    float       minClippingDistance;
    float       maxClippingDistance;
};

STRUCT_DEFINE(ZoneBySourceId)
{
    int     sourceId;
    Zone*   zone;
};

STRUCT_DEFINE(ClientListing)
{
    int     expansionId;
    Client* client;
};

Zone*   zone_create(R(ZC*) zc, int sourceId, int zoneId, int instId);
void    zone_destroy(R(ZC*) zc, R(Zone*) zone);

#define zone_type(zone) ((zone)->zoneType)
#define zone_sky_type(zone) ((zone)->skyType)
#define zone_gravity(zone) ((zone)->gravity)
#define zone_min_z(zone) ((zone)->minZ)
#define zone_min_clipping_distance(zone) ((zone)->minClippingDistance)
#define zone_max_clipping_distance(zone) ((zone)->maxClippingDistance)

void    zone_spawn_client(R(ZC*) zc, R(Zone*) zone, R(Client*) client);

void    zone_broadcast_packet(R(Zone*) zone, R(PacketBroadcast*) packet, R(Client*) ignore);
#define zone_broadcast_packet_to_all(zone, packet) zone_broadcast_packet((zone), (packet), NULL)

/* LuaJIT API */
EQP_API int         zone_get_source_id(R(Zone*) zone);
EQP_API uint16_t    zone_get_zone_id(R(Zone*) zone);
EQP_API uint16_t    zone_get_instance_id(R(Zone*) zone);
EQP_API const char* zone_get_short_name(R(Zone*) zone);
EQP_API const char* zone_get_long_name(R(Zone*) zone);

#endif//EQP_ZONE_H
