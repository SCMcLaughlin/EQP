
#ifndef EQP_ZONE_H
#define EQP_ZONE_H

#include "define.h"
#include "eqp_array.h"
#include "eqp_string.h"
#include "eqp_alloc.h"
#include "lua_object.h"
#include "mob.h"
#include "npc.h"
#include "npc_prototype.h"
#include "client.h"
#include "zone_id.h"
#include "structs.h"
#include "packet_broadcast.h"
#include "client_packet_trilogy_output.h"
#include "line_of_sight_map.h"

STRUCT_DECLARE(ZC);

STRUCT_DEFINE(Zone)
{
    // LuaObject must be the first member of this struct
    LuaObject       luaObj;
    int             sourceId;
    uint16_t        zoneId;
    uint16_t        instanceId;
    
    LineOfSightMap  losMap;
    
    Array*          mobsByEntityId; // entityId = array index + 1
    Array*          mobsByPosition;
    Array*          clientList;
    Array*          npcList;
    Array*          npcsPendingSpawn; // Holds NPCs that have been scheduled to spawn this cycle -- ensures that npcList is re-entrant
    
    Loc             safeSpot;
    FogStats        fogStats[4];
    uint8_t         zoneType;
    uint16_t        skyType;
    float           gravity;
    float           minZ;
    float           minClippingDistance;
    float           maxClippingDistance;
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

#define zone_type(zone) zone_get_zone_type_id((zone))
#define zone_sky_type(zone) zone_get_sky_id((zone))
#define zone_gravity(zone) zone_get_gravity((zone))
#define zone_set_min_z(zone, z) ((zone)->minZ = (z))
#define zone_min_z(zone) ((zone)->minZ)
#define zone_min_clipping_distance(zone) zone_get_min_clipping_distance((zone))
#define zone_max_clipping_distance(zone) zone_get_max_clipping_distance((zone))

EQP_API Npc*    zone_spawn_npc(ZC* zc, Zone* zone, NpcPrototype* proto, float x, float y, float z, float heading);

void    zone_spawn_client(ZC* zc, Zone* zone, Client* client);
void    zone_remove_client(Zone* zone, Client* client);
void    zone_mark_client_as_linkdead(Zone* zone, Client* client);

Mob*    zone_get_mob_by_entity_id(Zone* zone, uint32_t entityId);

void    zone_update_mob_position(Zone* zone, int index, float x, float y, float z);

void    zone_broadcast_packet(Zone* zone, PacketBroadcast* packet, Client* ignore);
#define zone_broadcast_packet_to_all(zone, packet) zone_broadcast_packet((zone), (packet), NULL)

/* LuaJIT API */
EQP_API int         zone_get_source_id(Zone* zone);
EQP_API uint16_t    zone_get_zone_id(Zone* zone);
EQP_API uint16_t    zone_get_instance_id(Zone* zone);
EQP_API const char* zone_get_short_name(Zone* zone);
EQP_API const char* zone_get_long_name(Zone* zone);
EQP_API uint8_t     zone_get_zone_type_id(Zone* zone);
EQP_API uint16_t    zone_get_sky_id(Zone* zone);
EQP_API float       zone_get_gravity(Zone* zone);
EQP_API float       zone_get_min_clipping_distance(Zone* zone);
EQP_API float       zone_get_max_clipping_distance(Zone* zone);
EQP_API float       zone_get_safe_spot_x(Zone* zone);
EQP_API float       zone_get_safe_spot_y(Zone* zone);
EQP_API float       zone_get_safe_spot_z(Zone* zone);
EQP_API float       zone_get_safe_spot_heading(Zone* zone);

EQP_API void        zone_set_zone_type_id(Zone* zone, uint32_t id);
EQP_API void        zone_set_sky_id(Zone* zone, uint32_t id);
EQP_API void        zone_set_gravity(Zone* zone, float value);
EQP_API void        zone_set_min_clipping_distance(Zone* zone, float value);
EQP_API void        zone_set_max_clipping_distance(Zone* zone, float value);
EQP_API void        zone_set_safe_spot(Zone* zone, float x, float y, float z, float heading);

EQP_API int         zone_are_points_in_line_of_sight(ZC* zc, Zone* zone, float x1, float y1, float z1, float x2, float y2, float z2);
EQP_API float       zone_get_best_z_for_loc(Zone* zone, float x, float y, float z);

#endif//EQP_ZONE_H
