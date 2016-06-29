
#include "client_packet_trilogy_input.h"
#include "client_packet_trilogy_output.h"
#include "zone_cluster.h"

static void client_trilogy_handle_op_zone_entry(Client* clientStub, Aligned* a)
{
    ProtocolHandler* handler;
    ZC* zc;
    
    if (aligned_remaining(a) < sizeof(Trilogy_ZoneEntryFromClient))
        return;
    
    handler = client_handler(clientStub);
    zc      = (ZC*)protocol_handler_basic(handler);
    
    // checksum
    aligned_advance(a, sizeof(uint32_t));
    // characterName
    zc_client_match_with_expected(zc, clientStub, handler, (const char*)aligned_current(a));
}

static void client_stub_recv_packet_trilogy(Client* client, uint16_t opcode, Aligned* a)
{
    if ((clock_milliseconds() - client->creationTimestamp) >= EQP_CLIENT_ZONE_IN_NO_AUTH_TIMEOUT)
    {
        protocol_handler_send_disconnect(client_handler(client));
        return;
    }
    
    switch (opcode)
    {
    case TrilogyOp_ZoneEntry:
        client_trilogy_handle_op_zone_entry(client, a);
        break;
    
    case TrilogyOp_SetDataRate:
        break;
    
    default:
        break;
    }
}

static void client_trilogy_echo_packet(Client* client, uint16_t opcode, Aligned* a)
{
    uint32_t length         = aligned_remaining(a);
    ZC* zc                  = client_zone_cluster(client);
    PacketTrilogy* packet   = packet_trilogy_create(B(zc), opcode, length);
    
    if (length)
        memcpy(packet_trilogy_data(packet), aligned_current(a), length);
    
    client_trilogy_schedule_packet_individual(client, packet);
}

static void client_trilogy_handle_op_inventory_request(Client* client)
{
    (void)client;
    //fixme: send items
}

static void client_trilogy_handle_op_zone_info_request(Client* client)
{
    Zone* zone              = client_zone(client);
    ZC* zc                  = client_zone_cluster(client);
    PacketTrilogy* packet   = client_trilogy_make_op_zone_info(zc, zone);
    
    client_trilogy_schedule_packet_individual(client, packet);
    
    client_trilogy_send_zero_length_packet(client, TrilogyOp_EnterZone);
    
    //fixme: send spawns, doors, objects
}

static void client_trilogy_handle_op_enter_zone(Client* client)
{
    PacketBroadcast broadcastSpawn;
    PacketTrilogy* packet;
    Zone* zone  = client_zone(client);
    ZC* zc      = client_zone_cluster(client);
    
    // Officially add the client to all the zone lists, so that we can get them their entityId
    zc_add_connected_client(zc, client);
    
    // Inform the client of their entityId
    packet = client_trilogy_make_op_spawn_appearance(zc, 0, Trilogy_SpawnAppearance_SetEntityId, client_entity_id(client));
    client_trilogy_schedule_packet_individual(client, packet);
    
    // Broadcast spawn packet, including to the client that is spawning
    packet = client_trilogy_make_op_spawn(zc, &client->mob);
    packet_broadcast_init(zc, &broadcastSpawn, packet_broadcast_encode_op_spawn, &client->mob);
    packet_broadcast_set_packet_for_expansion(&broadcastSpawn, ExpansionId_Trilogy, packet);
    zone_broadcast_packet_to_all(zone, &broadcastSpawn);
    
    // Send the client some things it expects
    client_trilogy_send_zeroed_packet_var_length(client, TrilogyOp_EnteredZoneUnknown, 8);
    client_trilogy_send_zero_length_packet(client, TrilogyOp_EnterZone);
    
    //send guildrank spawnappearance, if applicable
    //send gm spawnappearance, if applicable
    
    //calc all the client's stats
    //send hp and mana updates
}

void client_recv_packet_trilogy(void* vclient, uint16_t opcode, Aligned* a)
{
    Client* client = (Client*)vclient;
    
    printf("Opcode: 0x%04x, length: %u\n", opcode, aligned_remaining(a));
    
    if (client->isStubClient)
    {
        client_stub_recv_packet_trilogy(client, opcode, a);
        return;
    }
    
    switch (opcode)
    {
    case TrilogyOp_InventoryRequest:
        client_trilogy_handle_op_inventory_request(client);
        break;
    
    case TrilogyOp_ZoneInfoRequest:
        client_trilogy_handle_op_zone_info_request(client);
        break;
    
    case TrilogyOp_EnterZone:
        client_trilogy_handle_op_enter_zone(client);
        break;
    
    // Echo packets
    case TrilogyOp_ZoneInUnknown:
        client_trilogy_echo_packet(client, opcode, a);
        break;
    
    default:
        break;
    }
}
