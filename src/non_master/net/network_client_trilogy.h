
#ifndef EQP_NETWORK_CLIENT_TRILOGY_H
#define EQP_NETWORK_CLIENT_TRILOGY_H

#include "define.h"
#include "netcode.h"
#include "network_client.h"
#include "eqp_array.h"
#include "packet_trilogy.h"

STRUCT_DEFINE(OutputPacketTrilogy)
{
    uint16_t        header;
    uint16_t        seq;
    uint16_t        ackRequest;
    uint16_t        fragGroup;
    uint16_t        fragCount;
    uint8_t         ackCounterAlwaysOne;
    uint8_t         ackCounterRequest;
    PacketTrilogy*  packet;
};

STRUCT_DEFINE(NetworkClientTrilogy)
{
    NetworkClient   base;
    Array*          outputPackets;
    
    uint16_t        nextAckResponse;
    uint16_t        nextSeqToSend;
    
    uint32_t        sendFromIndex;
    uint16_t        sendFromFrag;
};

void        network_client_trilogy_init(R(UdpSocket*) sock, R(UdpClient*) udpClient, R(NetworkClientTrilogy*) client);
void        network_client_trilogy_deinit(R(NetworkClientTrilogy*) client);

void        network_client_trilogy_recv_ack_request(R(NetworkClientTrilogy*) client, uint16_t ack);

void        network_client_trilogy_schedule_packet(R(NetworkClientTrilogy*) client, R(OutputPacketTrilogy*) packet);
void        network_client_trilogy_send_queued(R(NetworkClientTrilogy*) client);

uint16_t    network_client_trilogy_get_next_seq_to_send_and_increment(R(NetworkClientTrilogy*) client, uint16_t by);

#define     network_client_trilogy_basic(cli) network_client_basic(&(cli)->base)

#endif//EQP_NETWORK_CLIENT_TRILOGY_H
