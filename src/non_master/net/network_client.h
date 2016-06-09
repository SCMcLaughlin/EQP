
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
    uint64_t    packetsSent;
    uint64_t    packetsReceived;
    uint32_t    accountId;
    //Array*      inputPackets;
};

void    network_client_init(R(UdpSocket*) sock, R(UdpClient*) udpClient, R(NetworkClient*) client);
void    network_client_deinit(R(NetworkClient*) client);

int     network_client_send_no_increment(R(NetworkClient*) client, R(const void*) data, uint32_t len);
int     network_client_send(R(NetworkClient*) client, R(const void*) data, uint32_t len);

#define network_client_basic(cli) ((cli)->basic)

#endif//EQP_NETWORK_CLIENT_H
