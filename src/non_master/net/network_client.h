
#ifndef EQP_NETWORK_CLIENT_H
#define EQP_NETWORK_CLIENT_H

#include "define.h"
#include "netcode.h"
#include "eqp_array.h"
//#include "packet_input.h"

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(UdpSocket);
STRUCT_DECLARE(UdpClient);

STRUCT_DEFINE(NetworkClient)
{
    int         socketFd;
    IpAddress   address;
    Basic*      basic;
    UdpSocket*  udpSocket;
    uint32_t    udpClientIndex;
    uint64_t    packetsSent;
    uint64_t    packetsReceived;
    uint32_t    accountId;
    //Array*      inputPackets;
};

void    network_client_init(R(UdpSocket*) sock, R(UdpClient*) udpClient, R(NetworkClient*) client, uint32_t index);
void    network_client_deinit(R(NetworkClient*) client);

int     network_client_send_no_increment(R(NetworkClient*) client, R(const void*) data, uint32_t len);
int     network_client_send(R(NetworkClient*) client, R(const void*) data, uint32_t len);

#define network_client_basic(cli) ((cli)->basic)
#define network_client_udp_socket(cli) ((cli)->udpSocket)

#define network_client_udp_client_index(cli) ((cli)->udpClientIndex)
#define network_client_update_udp_client_index(cli, i) ((cli)->udpClientIndex = (i))
void    network_client_flag_connection_as_dead(R(NetworkClient*) client);

#endif//EQP_NETWORK_CLIENT_H
