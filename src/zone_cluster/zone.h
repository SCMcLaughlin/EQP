
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
#include "client_packet_trilogy_output.h"

STRUCT_DECLARE(ZC);

STRUCT_DEFINE(Zone)
{
    // LuaObject must be the first member of this struct
    LuaObject   luaObj;
    int         sourceId;
    uint16_t    zoneId;
    uint16_t    instanceId;
    
    Array*      mobsByEntityId; // entityId = array index + 1
    Array*      mobsByPosition;
    Array*      clientList;
    Array*      npcList;
    Array*      npcsPendingSpawn; // Holds NPCs that have been scheduled to spawn this cycle -- ensures that npcList is re-entrant
    
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
    int     isLinkdead;
    Client* client;
};

Zone*   zone_create(ZC* zc, int sourceId, int zoneId, int instId);
void    zone_destroy(ZC* zc, Zone* zone);

#define zone_type(zone) ((zone)->zoneType)
#define zone_sky_type(zone) ((zone)->skyType)
#define zone_gravity(zone) ((zone)->gravity)
#define zone_min_z(zone) ((zone)->minZ)
#define zone_min_clipping_distance(zone) ((zone)->minClippingDistance)
#define zone_max_clipping_distance(zone) ((zone)->maxClippingDistance)

void    zone_spawn_client(ZC* zc, Zone* zone, Client* client);
void    zone_remove_client(Zone* zone, Client* client);
void    zone_mark_client_as_linkdead(Zone* zone, Client* client);

void    zone_broadcast_packet(Zone* zone, PacketBroadcast* packet, Client* ignore);
#define zone_broadcast_packet_to_all(zone, packet) zone_broadcast_packet((zone), (packet), NULL)

/* LuaJIT API */
EQP_API int         zone_get_source_id(Zone* zone);
EQP_API uint16_t    zone_get_zone_id(Zone* zone);
EQP_API uint16_t    zone_get_instance_id(Zone* zone);
EQP_API const char* zone_get_short_name(Zone* zone);
EQP_API const char* zone_get_long_name(Zone* zone);

#endif//EQP_ZONE_H
