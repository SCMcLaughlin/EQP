
#ifndef EQP_NETWORK_CLIENT_TRILOGY_H
#define EQP_NETWORK_CLIENT_TRILOGY_H

#include "define.h"
#include "netcode.h"
#include "network_client.h"
#include "eqp_array.h"
#include "packet_trilogy.h"
#include "ack_common.h"

#define EQP_TRILOGY_RESEND_MILLISECONDS 250

STRUCT_DEFINE(OutputPacketTrilogy)
{
    uint16_t        header;
    uint16_t        ackRequest;
    uint16_t        fragGroup;
    uint16_t        fragCount;
    uint8_t         ackCounterAlwaysOne;
    uint8_t         ackCounterRequest;
    uint64_t        ackTimestamp;
    PacketTrilogy*  packet;
};

STRUCT_DEFINE(NetworkClientTrilogy)
{
    NetworkClient   base;
    Array*          outputPackets;
    
    uint16_t        nextAckResponse;
    uint16_t        nextAckRequestExpected; //fixme: rename these to better reflect what they do; basically: expected is the beginning
    uint16_t        nextAckRequestCheck;    //of the received packet frontier, check is the end, and response = check, but is set to 0 when the ack response has been sent
    uint16_t        nextSeqToSend;
    uint16_t        lastAckReceived;
    
    uint32_t        sendFromIndex;
};

void        network_client_trilogy_init(UdpSocket* sock, UdpClient* udpClient, NetworkClientTrilogy* client, uint32_t index);
void        network_client_trilogy_deinit(NetworkClientTrilogy* client);

void        network_client_trilogy_recv_ack_response(NetworkClientTrilogy* client, uint16_t ack);
void        network_client_trilogy_recv_ack_request(NetworkClientTrilogy* client, uint16_t ack, int isFirstPacket);

void        network_client_trilogy_schedule_packet(NetworkClientTrilogy* client, OutputPacketTrilogy* packet);
void        network_client_trilogy_send_pure_ack(NetworkClientTrilogy* client, uint16_t ackResponse);
void        network_client_trilogy_send_disconnect(NetworkClientTrilogy* client);
void        network_client_trilogy_send_queued(NetworkClientTrilogy* client);

#define     network_client_trilogy_next_ack_response(cli) ((cli)->nextAckResponse)
#define     network_client_trilogy_set_next_ack_response(cli, ack) ((cli)->nextAckResponse = (ack))
#define     network_client_trilogy_next_ack_request_expected(cli) ((cli)->nextAckRequestExpected)
#define     network_client_trilogy_set_next_ack_request_expected(cli, ack) ((cli)->nextAckRequestExpected = (ack))

#define     network_client_trilogy_basic(cli) network_client_basic(&(cli)->base)
#define     network_client_trilogy_ip_address(cli) network_client_ip_address(&(cli)->base)
#define     network_client_trilogy_update_index(cli, i) network_client_update_udp_client_index(&(cli)->base, i)
#define     network_client_trilogy_flag_connection_as_dead(cli) network_client_flag_connection_as_dead(&(cli)->base)
#define     network_client_trilogy_increment_packets_received(cli) network_client_increment_packets_received(&(cli)->base)
#define     network_client_trilogy_send_keep_alive_ack(cli) network_client_trilogy_send_pure_ack((cli), (cli)->nextAckRequestCheck)

#endif//EQP_NETWORK_CLIENT_TRILOGY_H
