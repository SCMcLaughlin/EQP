
#ifndef EQP_PROTOCOL_HANDLER_TRILOGY_H
#define EQP_PROTOCOL_HANDLER_TRILOGY_H

#include "define.h"
#include "netcode.h"
#include "crc.h"
#include "aligned.h"
#include "ack_mgr_trilogy.h"
#include "network_interface.h"

STRUCT_DECLARE(UdpSocket);
STRUCT_DECLARE(UdpClient);
STRUCT_DECLARE(ProtocolHandler);

STRUCT_DEFINE(ProtocolHandlerTrilogy)
{
    AckMgrTrilogy   ackMgr;
    void*           clientObject;
};

void    protocol_handler_trilogy_init(R(UdpSocket*) sock, R(UdpClient*) client, R(ProtocolHandler*) handler, uint32_t index);
void    protocol_handler_trilogy_deinit(R(ProtocolHandlerTrilogy*) handler);

void    protocol_handler_trilogy_recv(R(ProtocolHandlerTrilogy*) handler, R(byte*) data, int len);

#define protocol_handler_trilogy_basic(handler) ack_mgr_trilogy_basic(&(handler)->ackMgr)
#define protocol_handler_trilogy_client_object(handler) ((handler)->clientObject)
#define protocol_handler_trilogy_ip_address(handler) ack_mgr_trilogy_ip_address(&(handler)->ackMgr)

#define protocol_handler_trilogy_set_client_object(handler, obj) ((handler)->clientObject = (obj))

#define protocol_handler_trilogy_update_index(handler, i) ack_mgr_trilogy_update_index(&(handler)->ackMgr, i)
#define protocol_handler_trilogy_flag_connection_as_dead(handler) ack_mgr_trilogy_flag_connection_as_dead(&(handler)->ackMgr)
#define protocol_handler_trilogy_increment_packets_received(handler) ack_mgr_trilogy_increment_packets_received(&(handler)->ackMgr)

#define protocol_handler_trilogy_schedule_packet(handler, packet) protocol_handler_trilogy_schedule_packet_opt(handler, packet, 0)
#define protocol_handler_trilogy_schedule_packet_no_ack(handler, packet) protocol_handler_trilogy_schedule_packet_opt(handler, packet, 1)
#define protocol_handler_trilogy_schedule_packet_opt(handler, packet, noAck) ack_mgr_trilogy_schedule_packet((&(handler)->ackMgr), (packet), (noAck))
#define protocol_handler_trilogy_send_queued(handler) ack_mgr_trilogy_send_queued((&(handler)->ackMgr))
#define protocol_handler_trilogy_send_pure_ack(handler, ack) ack_mgr_trilogy_send_pure_ack(&(handler)->ackMgr, (ack))
#define protocol_handler_trilogy_send_disconnect(handler) ack_mgr_trilogy_send_disconnect(&(handler)->ackMgr)

#endif//EQP_PROTOCOL_HANDLER_TRILOGY_H
