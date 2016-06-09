
#ifndef EQP_PROTOCOL_HANDLER_H
#define EQP_PROTOCOL_HANDLER_H

#include "define.h"
#include "netcode.h"
#include "crc.h"
#include "aligned.h"
#include "packet_trilogy.h"
#include "protocol_handler_standard.h"
#include "protocol_handler_trilogy.h"

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(UdpSocket);
STRUCT_DECLARE(UdpClient);

STRUCT_DEFINE(ProtocolHandler)
{
    int isTrilogy;
    union
    {
        ProtocolHandlerStandard standard;
        ProtocolHandlerTrilogy  trilogy;
    };
};

ProtocolHandler*    protocol_handler_create(R(Basic*) basic);
void                protocol_handler_destroy(R(ProtocolHandler*) handler);

Basic*              protocol_handler_basic(R(ProtocolHandler*) handler);

void                protocol_handler_recv(R(ProtocolHandler*) handler, R(byte*) data, int len);
void                protocol_handler_check_first_packet(R(UdpSocket*) sock, R(UdpClient*) client, R(ProtocolHandler*) handler, R(byte*) data, int len);
void                protocol_handler_send_queued(R(ProtocolHandler*) handler);
void                protocol_handler_schedule_packet_opt(R(ProtocolHandler*) handler, R(void*) packet, int noAckRequest);
#define             protocol_handler_schedule_packet(handler, packet) protocol_handler_schedule_packet_opt((handler), (packet), 0)
#define             protocol_handler_schedule_packet_no_ack(handler, packet) protocol_handler_schedule_packet_opt((handler), (packet), 1)

#endif//EQP_PROTOCOL_HANDLER_H
