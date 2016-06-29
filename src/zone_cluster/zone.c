
#include "zone.h"
#include "zone_cluster.h"

Zone* zone_create(ZC* zc, int sourceId, int zoneId, int instId)
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

void zone_destroy(ZC* zc, Zone* zone)
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
    
    if (zone->clientList)
    {
        array_destroy(zone->clientList);
        zone->clientList = NULL;
    }
    
    zc_lua_destroy_object(zc, &zone->luaObj);
    
    free(zone);
}

static void zone_spawn_mob(ZC* zc, Zone* zone, Mob* mob)
{
    MobByPosition pos;
    Mob** entityArray   = array_data_type(zone->mobsByEntityId, Mob*);
    uint32_t n          = array_count(zone->mobsByEntityId);
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

void zone_spawn_client(ZC* zc, Zone* zone, Client* client)
{
    ClientListing listing;
    
    zone_spawn_mob(zc, zone, &client->mob);
    
    listing.expansionId = client_expansion(client);
    listing.isLinkdead  = false;
    listing.client      = client;
    
    client_set_zone_index(client, array_count(zone->clientList));
    array_push_back(B(zc), &zone->clientList, &listing);
}

void zone_remove_client(Zone* zone, Client* client)
{
    int cIndex      = client_zone_index(client);
    int mIndex      = mob_zone_index(&client->mob);
    int entityId    = client_entity_id(client);
    
    if (array_swap_and_pop(zone->clientList, cIndex))
    {
        ClientListing* listing = array_get_type(zone->clientList, cIndex, ClientListing);
        client_set_zone_index(listing->client, cIndex);
    }
    
    if (array_swap_and_pop(zone->mobsByPosition, mIndex))
    {
        MobByPosition* pos = array_get_type(zone->mobsByPosition, mIndex, MobByPosition);
        mob_set_zone_index(pos->mob, mIndex);
    }
    
    if (entityId)
    {
        Mob** ent   = array_get_type(zone->mobsByEntityId, entityId - 1, Mob*);
        *ent        = NULL;
    }
    
    //fixme: broadcast despawn (add zone_despawn_mob?), and send the client whatever they need for removal
}

void zone_mark_client_as_linkdead(Zone* zone, Client* client)
{
    int index               = client_zone_index(client);
    ClientListing* listing  = array_get_type(zone->clientList, index, ClientListing);
    
    if (!listing)
        return;
    
    listing->isLinkdead = true;
    
    //fixme: broadcast this client's linkdead status
}

void zone_broadcast_packet(Zone* zone, PacketBroadcast* packetBroadcast, Client* ignore)
{
    ClientListing* array    = array_data_type(zone->clientList, ClientListing);
    uint32_t n              = array_count(zone->clientList);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        ClientListing* listing = &array[i];
        
        if (listing->client == ignore || listing->isLinkdead)
            continue;
        
        if (listing->expansionId == ExpansionId_Trilogy)
        {
            PacketTrilogy* packet = packet_broadcast_get_trilogy(packetBroadcast);
            
            if (packet)
                client_trilogy_schedule_packet_broadcast(listing->client, packet);
        }
        else
        {
            
        }
    }
}

/* LuaJIT API */

int zone_get_source_id(Zone* zone)
{
    return zone->sourceId;
}

uint16_t zone_get_zone_id(Zone* zone)
{
    return zone->zoneId;
}

uint16_t zone_get_instance_id(Zone* zone)
{
    return zone->instanceId;
}

const char* zone_get_short_name(Zone* zone)
{
    return zone_short_name_by_id(zone->zoneId);
}

const char* zone_get_long_name(Zone* zone)
{
    return zone_long_name_by_id(zone->zoneId);
}
