
#ifndef EQP_ACK_MGR_TRILOGY_H
#define EQP_ACK_MGR_TRILOGY_H

#include "define.h"
#include "netcode.h"
#include "crc.h"
#include "random.h"
#include "eqp_array.h"
#include "network_client_trilogy.h"
#include "packet_trilogy.h"

STRUCT_DECLARE(UdpSocket);
STRUCT_DECLARE(UdpClient);

STRUCT_DEFINE(AckMgrTrilogy)
{
    NetworkClientTrilogy    client;
    
    uint16_t    nextAckToRequest;
    uint16_t    nextSeqToReceive;
    uint16_t    nextFragGroup;
    
    uint8_t     ackCounterAlwaysOne; // If this turns out to NOT always be one, then this field will have a point; otherwise, remove later
    uint8_t     ackCounterRequest;
};

void    ack_mgr_trilogy_init(R(UdpSocket*) sock, R(UdpClient*) client, R(AckMgrTrilogy*) ackMgr);
void    ack_mgr_trilogy_deinit(R(AckMgrTrilogy*) ackMgr);

#define ack_mgr_trilogy_recv_ack_response(ackMgr, ack) network_client_trilogy_recv_ack_response((&(ackMgr)->client), (ack))
#define ack_mgr_trilogy_recv_ack_request(ackMgr, ack) network_client_trilogy_recv_ack_request((&(ackMgr)->client), (ack))

void    ack_mgr_trilogy_schedule_packet(R(AckMgrTrilogy*) ackMgr, R(PacketTrilogy*) packet, int noAckRequest);
#define ack_mgr_trilogy_send_queued(ackMgr) network_client_trilogy_send_queued((&(ackMgr)->client))

#define ack_mgr_trilogy_basic(ack) network_client_trilogy_basic(&(ack)->client)

#endif//EQP_ACK_MGR_TRILOGY_H
