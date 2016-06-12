
#ifndef EQP_ACK_MGR_STANDARD_H
#define EQP_ACK_MGR_STANDARD_H

#include "define.h"
#include "netcode.h"
#include "crc.h"
#include "network_client_standard.h"

STRUCT_DECLARE(UdpSocket);
STRUCT_DECLARE(UdpClient);

STRUCT_DEFINE(AckMgrStandard)
{
    NetworkClientStandard   client;
};

void    ack_mgr_standard_init(R(UdpSocket*) sock, R(UdpClient*) client, R(AckMgrStandard*) ackMgr, uint32_t index);
void    ack_mgr_standard_deinit(R(AckMgrStandard*) ackMgr);

#define ack_mgr_standard_basic(ack) network_client_standard_basic(&(ack)->client)

#define ack_mgr_standard_increment_packets_received(ack) network_client_standard_increment_packets_received(&(ack)->client)

#endif//EQP_ACK_MGR_STANDARD_H
