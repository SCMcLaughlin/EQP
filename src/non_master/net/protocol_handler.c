
#include "protocol_handler.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"
#include "udp_socket.h"

ProtocolHandler* protocol_handler_create(R(Basic*) basic)
{
    ProtocolHandler* handler = eqp_alloc_type(basic, ProtocolHandler);
    
    handler->isTrilogy = -1;
    
    return handler;
}

void protocol_handler_destroy(R(ProtocolHandler*) handler)
{
    int isTrilogy = handler->isTrilogy;
    
    if (isTrilogy == 0)
        protocol_handler_standard_deinit(&handler->standard);
    else if (isTrilogy == 1)
        protocol_handler_trilogy_deinit(&handler->trilogy);
    
    free(handler);
}

Basic* protocol_handler_basic(R(ProtocolHandler*) handler)
{
    R(Basic*) basic;
    
    /*if (handler->isTrilogy == 0)
        protocol_handler_standard_schedule_packet(&handler->standard);
    else*/
        basic = protocol_handler_trilogy_basic(&handler->trilogy);
    
    return basic;
}

void protocol_handler_recv(R(ProtocolHandler*) handler, R(byte*) data, int len)
{
    if (handler->isTrilogy == 0)
        protocol_handler_standard_recv(&handler->standard, data, len);
    else
        protocol_handler_trilogy_recv(&handler->trilogy, data, len);
}

void protocol_handler_check_first_packet(R(UdpSocket*) sock, R(UdpClient*) client, R(ProtocolHandler*) handler, R(byte*) data, int len)
{
    Aligned a;
    uint16_t header;
    
    aligned_init(udp_socket_basic(sock), &a, data, len);
    header = aligned_read_uint16(&a);
    
    if (header == (PacketTrilogyHasAckRequest | PacketTrilogyHasAckCounter | PacketTrilogyIsFirstPacket))
    {
        len -= sizeof(uint32_t);
        aligned_advance(&a, len - sizeof(uint16_t));
        
        if (crc_calc32_network(data, len) == aligned_read_uint32(&a))
        {
            handler->isTrilogy = 1;
            protocol_handler_trilogy_init(sock, client, handler);
            return;
        }
    }
    
    handler->isTrilogy = 0;
    protocol_handler_standard_init(sock, client, handler);
}

void protocol_handler_send_queued(R(ProtocolHandler*) handler)
{
    /*if (handler->isTrilogy == 0)
        protocol_handler_standard_send_queued(&handler->standard);
    else*/
        protocol_handler_trilogy_send_queued(&handler->trilogy);
}

void protocol_handler_schedule_packet_opt(R(ProtocolHandler*) handler, R(void*) packet, int noAckRequest)
{
    /*if (handler->isTrilogy == 0)
        protocol_handler_standard_schedule_packet(&handler->standard);
    else*/
        protocol_handler_trilogy_schedule_packet_opt(&handler->trilogy, (PacketTrilogy*)packet, noAckRequest);
}
