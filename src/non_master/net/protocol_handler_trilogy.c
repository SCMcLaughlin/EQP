
#include "protocol_handler_trilogy.h"
#include "protocol_handler.h"
#include "udp_socket.h"
#include "udp_client.h"
#include "eqp_basic.h"

#define MIN_PACKET_LENGTH 10

void protocol_handler_trilogy_init(R(UdpSocket*) sock, R(UdpClient*) client, R(ProtocolHandler*) handler, uint32_t index)
{
    ack_mgr_trilogy_init(sock, client, &handler->trilogy.ackMgr, index);
    
    handler->trilogy.clientObject = client_create_from_new_connection_trilogy(handler);
}

void protocol_handler_trilogy_deinit(R(ProtocolHandlerTrilogy*) handler)
{
    ack_mgr_trilogy_deinit(&handler->ackMgr);
}

void protocol_handler_trilogy_recv(R(ProtocolHandlerTrilogy*) handler, R(byte*) data, int len)
{
    Aligned aligned;
    R(Aligned*) a = &aligned;
    uint16_t header;
    uint16_t seq;
    uint16_t opcode;
    R(AckMgrTrilogy*) ackMgr;
    R(void*) clientObject;
    
    if (len < MIN_PACKET_LENGTH)
        return;
    
    ackMgr          = &handler->ackMgr;
    clientObject    = handler->clientObject;
    
    aligned_init(protocol_handler_trilogy_basic(handler), a, data, len);
    
    // Every trilogy packet has a crc, check it first
    len -= sizeof(uint32_t);
    aligned_advance(a, len);
    
    if (crc_calc32_network(data, len) != aligned_read_uint32(a))
        return;
    
    aligned_reset(a);
    aligned_reduce_size(a, sizeof(uint32_t));
    
    header  = aligned_read_uint16(a);
    seq     = aligned_read_uint16(a);
    
    if (header & (PacketTrilogyIsClosing | PacketTrilogyIsClosing2))
    {
        protocol_handler_trilogy_flag_connection_as_dead(handler);
        client_on_disconnect(clientObject, false);
    }
    
    if (header & PacketTrilogyHasAckResponse)
    {
        uint16_t ackResponse = aligned_read_uint16(a);
        ack_mgr_trilogy_recv_ack_response(ackMgr, ackResponse);
        
        // Pure ack?
        if (aligned_remaining(a) == 0)
            return;
    }
    
    // Weird things we don't care about but need to skip when present
    if (header & PacketTrilogyUnknownBit11)
        aligned_advance(a, sizeof(uint16_t));
    
    if (header & PacketTrilogyUnknownBit12)
        aligned_advance(a, sizeof(byte));
    
    if (header & PacketTrilogyUnknownBit13)
        aligned_advance(a, sizeof(uint16_t));
    
    if (header & PacketTrilogyUnknownBit14)
        aligned_advance(a, sizeof(uint32_t));
    
    if (header & PacketTrilogyUnknownBit15)
        aligned_advance(a, sizeof(uint64_t));
    
    // Back to your regularly-scheduled fields
    if (header & PacketTrilogyHasAckRequest)
    {
        uint16_t ackRequest = aligned_read_uint16(a);
        ack_mgr_trilogy_recv_ack_request(ackMgr, ackRequest);
    }
    
    if (header & PacketTrilogyIsFragment)
    {
        
    }
    
    if (header & PacketTrilogyHasAckCounter)
        aligned_advance(a, (header & PacketTrilogyHasAckRequest) ? (sizeof(uint8_t) * 2) : sizeof(uint8_t)); // Do we have any reason to care about these?
    
    if (aligned_remaining(a) >= sizeof(uint16_t))
        opcode = aligned_read_uint16(a);
    else
        opcode = 0;
    
    if (opcode)
        client_recv_packet_trilogy(clientObject, opcode, a);
}

#undef MIN_PACKET_LENGTH
