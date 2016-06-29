
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

void    ack_mgr_standard_init(UdpSocket* sock, UdpClient* client, AckMgrStandard* ackMgr, uint32_t index);
void    ack_mgr_standard_deinit(AckMgrStandard* ackMgr);

#define ack_mgr_standard_basic(ack) network_client_standard_basic(&(ack)->client)
#define ack_mgr_standard_update_index(ack, i) network_client_standard_update_index(&(ack)->client, i)
#define ack_mgr_standard_flag_connection_as_dead(ack) network_client_standard_flag_connection_as_dead(&(ack)->client)
#define ack_mgr_standard_increment_packets_received(ack) network_client_standard_increment_packets_received(&(ack)->client)

#define ack_mgr_standard_send_immediate(ackMgr, data, len) network_client_standard_send_immediate(&(ackMgr)->client, (data), (len))

#endif//EQP_ACK_MGR_STANDARD_H
