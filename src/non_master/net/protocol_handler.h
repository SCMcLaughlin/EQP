
#ifndef EQP_PROTOCOL_HANDLER_H
#define EQP_PROTOCOL_HANDLER_H

#include "define.h"
#include "netcode.h"
#include "crc.h"
#include "aligned.h"
#include "packet_trilogy.h"
#include "protocol_handler_standard.h"
#include "protocol_handler_trilogy.h"
#include <stdatomic.h>

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(UdpSocket);
STRUCT_DECLARE(UdpClient);

STRUCT_DEFINE(ProtocolHandler)
{
    int         isTrilogy;
    atomic_int  refCount;
    union
    {
        ProtocolHandlerStandard standard;
        ProtocolHandlerTrilogy  trilogy;
    };
};

ProtocolHandler*    protocol_handler_create(R(Basic*) basic);
#define             protocol_handler_grab(handler) atomic_fetch_add(&(handler)->refCount, 1)
void                protocol_handler_drop(R(ProtocolHandler*) handler);

Basic*              protocol_handler_basic(R(ProtocolHandler*) handler);
void                protocol_handler_update_index(R(ProtocolHandler*) handler, uint32_t index);
void*               protocol_handler_client_object(R(ProtocolHandler*) handler);
IpAddress*          protocol_handler_ip_address(R(ProtocolHandler*) handler);

void                protocol_handler_set_client_object(R(ProtocolHandler*) handler, R(void*) clientObject);

void                protocol_handler_recv(R(ProtocolHandler*) handler, R(byte*) data, int len);
void                protocol_handler_check_first_packet(R(UdpSocket*) sock, R(UdpClient*) client, R(ProtocolHandler*) handler, R(byte*) data, int len, uint32_t index);
void                protocol_handler_send_queued(R(ProtocolHandler*) handler);
void                protocol_handler_schedule_packet_opt(R(ProtocolHandler*) handler, R(void*) packet, int noAckRequest);
#define             protocol_handler_schedule_packet(handler, packet) protocol_handler_schedule_packet_opt((handler), (packet), 0)
#define             protocol_handler_schedule_packet_no_ack(handler, packet) protocol_handler_schedule_packet_opt((handler), (packet), 1)
void                protocol_handler_send_disconnect(R(ProtocolHandler*) handler);

#endif//EQP_PROTOCOL_HANDLER_H
