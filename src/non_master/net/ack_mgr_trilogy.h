
#ifndef EQP_ACK_MGR_TRILOGY_H
#define EQP_ACK_MGR_TRILOGY_H

#include "define.h"
#include "netcode.h"
#include "crc.h"
#include "random.h"
#include "aligned.h"
#include "eqp_array.h"
#include "network_client_trilogy.h"
#include "packet_trilogy.h"
#include "ack_common.h"

STRUCT_DECLARE(UdpSocket);
STRUCT_DECLARE(UdpClient);

STRUCT_DEFINE(InputPacketTrilogy)
{
    uint16_t    ackRequest;
    uint16_t    fragCount;
    uint16_t    opcode;
    uint32_t    length;
    byte*       data;
};

STRUCT_DEFINE(AckMgrTrilogy)
{
    NetworkClientTrilogy    client;
    
    uint16_t    nextAckToRequest;
    uint16_t    nextFragGroup;
    
    uint8_t     ackCounterAlwaysOne; // If this turns out to NOT always be one, then this field will have a point; otherwise, remove later
    uint8_t     ackCounterRequest;
    
    Array*      inputPackets;
};

void    ack_mgr_trilogy_init(R(UdpSocket*) sock, R(UdpClient*) client, R(AckMgrTrilogy*) ackMgr, uint32_t index);
void    ack_mgr_trilogy_deinit(R(AckMgrTrilogy*) ackMgr);

#define ack_mgr_trilogy_recv_ack_response(ackMgr, ack) network_client_trilogy_recv_ack_response((&(ackMgr)->client), (ack))
#define ack_mgr_trilogy_recv_ack_request(ackMgr, ack, isFirstPacket) network_client_trilogy_recv_ack_request((&(ackMgr)->client), (ack), (isFirstPacket))
void    ack_mgr_trilogy_recv_packet(R(AckMgrTrilogy*) ackMgr, R(Aligned*) a, R(void*) clientObject, uint16_t opcode, uint16_t ackRequest, uint16_t fragCount);

void    ack_mgr_trilogy_schedule_packet(R(AckMgrTrilogy*) ackMgr, R(PacketTrilogy*) packet, int noAckRequest);
#define ack_mgr_trilogy_send_queued(ackMgr) network_client_trilogy_send_queued((&(ackMgr)->client))
#define ack_mgr_trilogy_send_pure_ack(ackMgr, ack) network_client_trilogy_send_pure_ack(&(ackMgr)->client, (ack))
#define ack_mgr_trilogy_send_disconnect(ackMgr) network_client_trilogy_send_disconnect(&(ackMgr)->client)

#define ack_mgr_trilogy_next_ack_response(ack) network_client_trilogy_next_ack_response(&(ack)->client)
#define ack_mgr_trilogy_next_ack_request_expected(ack) network_client_trilogy_next_ack_request_expected(&(ack)->client)

#define ack_mgr_trilogy_basic(ack) network_client_trilogy_basic(&(ack)->client)
#define ack_mgr_trilogy_ip_address(ack) network_client_trilogy_ip_address(&(ack)->client)
#define ack_mgr_trilogy_update_index(ack, i) network_client_trilogy_update_index(&(ack)->client, i)
#define ack_mgr_trilogy_flag_connection_as_dead(ack) network_client_trilogy_flag_connection_as_dead(&(ack)->client)
#define ack_mgr_trilogy_increment_packets_received(ack) network_client_trilogy_increment_packets_received(&(ack)->client)

#endif//EQP_ACK_MGR_TRILOGY_H
