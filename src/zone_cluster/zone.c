
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
    
    zone->mobsByEntityId    = array_create_type(B(zc), Mob*);
    zone->mobsByPosition    = array_create_type(B(zc), MobByPosition);
    zone->clientList        = array_create_type(B(zc), ClientListing);
    
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

static void zone_spawn_mob(R(ZC*) zc, R(Zone*) zone, R(Mob*) mob)
{
    MobByPosition pos;
    R(Mob**) entityArray    = array_data_type(zone->mobsByEntityId, Mob*);
    uint32_t n              = array_count(zone->mobsByEntityId);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        if (entityArray[i] == NULL)
        {
            entityArray[i] = mob;
            mob_set_entity_id(mob, i + 1);
            goto found_space_entity;
        }
    }
    
    // If we reach here, no gaps were found
    array_push_back(B(zc), &zone->mobsByEntityId, (void*)&mob);
    mob_set_entity_id(mob, n + 1);
    
found_space_entity:
    
    pos.x   = mob_x(mob);
    pos.y   = mob_y(mob);
    pos.z   = mob_z(mob);
    pos.mob = mob;
    
    switch (mob_get_type(mob))
    {
    case MobType_Npc:
        pos.aggroRadiusSquared = 0.0f; //fixme: handle this
        break;
    
    case MobType_Client:
        pos.aggroRadiusSquared = -1.0f;
        break;
    
    case MobType_Pet:
        pos.aggroRadiusSquared = -2.0f;
        break;
    }
    
    n = array_count(zone->mobsByPosition);
    array_push_back(B(zc), &zone->mobsByPosition, &pos);
    mob_set_zone_index(mob, n);
}

void zone_spawn_client(R(ZC*) zc, R(Zone*) zone, R(Client*) client)
{
    ClientListing listing;
    
    zone_spawn_mob(zc, zone, &client->mob);
    
    listing.expansionId = client_expansion(client);
    listing.client      = client;
    
    client_set_zone_index(client, array_count(zone->clientList));
    array_push_back(B(zc), &zone->clientList, &listing);
}

void zone_broadcast_packet(R(Zone*) zone, R(PacketBroadcast*) packetBroadcast, R(Client*) ignore)
{
    R(ClientListing*) array = array_data_type(zone->clientList, ClientListing);
    uint32_t n              = array_count(zone->clientList);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        R(ClientListing*) listing = &array[i];
        
        if (listing->client == ignore)
            continue;
        
        if (listing->expansionId == ExpansionId_Trilogy)
        {
            R(PacketTrilogy*) packet = packet_broadcast_get_trilogy(packetBroadcast);
            
            if (packet)
                client_trilogy_schedule_packet_broadcast(listing->client, packet);
        }
        else
        {
            
        }
    }
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
