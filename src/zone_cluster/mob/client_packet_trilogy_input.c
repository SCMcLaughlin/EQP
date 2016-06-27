
#include "client_packet_trilogy_input.h"
#include "client_packet_trilogy_output.h"
#include "zone_cluster.h"

static void client_trilogy_handle_op_zone_entry(R(Client*) clientStub, R(Aligned*) a)
{
    R(ProtocolHandler*) handler;
    R(ZC*) zc;
    
    if (aligned_remaining(a) < sizeof(Trilogy_ZoneEntry))
        return;
    
    handler = client_handler(clientStub);
    zc      = (ZC*)protocol_handler_basic(handler);
    
    // checksum
    aligned_advance(a, sizeof(uint32_t));
    // characterName
    zc_client_match_with_expected(zc, clientStub, handler, (const char*)aligned_current(a));
}

static void client_stub_recv_packet_trilogy(R(Client*) client, uint16_t opcode, R(Aligned*) a)
{
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

static void client_trilogy_echo_packet(R(Client*) client, uint16_t opcode, R(Aligned*) a)
{
    uint32_t length             = aligned_remaining(a);
    R(ZC*) zc                   = client_zone_cluster(client);
    R(PacketTrilogy*) packet    = packet_trilogy_create(B(zc), opcode, length);
    
    if (length)
        memcpy(packet_trilogy_data(packet), aligned_current(a), length);
    
    client_trilogy_schedule_packet_individual(client, packet);
}

static void client_trilogy_handle_op_inventory_request(R(Client*) client)
{
    (void)client;
    //fixme: send items
}

static void client_trilogy_handle_op_zone_info_request(R(Client*) client)
{
    R(Zone*) zone               = client_zone(client);
    R(ZC*) zc                   = client_zone_cluster(client);
    R(PacketTrilogy*) packet    = client_trilogy_make_op_zone_info(zc, zone);
    
    client_trilogy_schedule_packet_individual(client, packet);
}

void client_recv_packet_trilogy(R(void*) vclient, uint16_t opcode, R(Aligned*) a)
{
    R(Client*) client = (Client*)vclient;
    
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
    
    // Echo packets
    case TrilogyOp_ZoneInUnknownA:
        client_trilogy_echo_packet(client, opcode, a);
        break;
    
    default:
        break;
    }
}
