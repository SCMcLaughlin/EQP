
#include "client_packet_trilogy.h"
#include "zone_cluster.h"

static void zc_trilogy_handle_op_zone_entry(R(Client*) clientStub, R(Aligned*) a)
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

void client_recv_packet_trilogy(R(void*) vclient, uint16_t opcode, R(Aligned*) a)
{
    R(Client*) client = (Client*)vclient;
    
    printf("Opcode: 0x%04x, length: %u\n", opcode, aligned_remaining(a));
    
    switch (opcode)
    {
    case TrilogyOp_ZoneEntry:
        zc_trilogy_handle_op_zone_entry(client, a);
        break;
    
    /* Ignored opcodes */
    case TrilogyOp_SetDataRate:
        break;
    
    default:
        break;
    }
}
