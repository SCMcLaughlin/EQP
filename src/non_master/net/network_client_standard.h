
#ifndef EQP_NETWORK_CLIENT_STANDARD_H
#define EQP_NETWORK_CLIENT_STANDARD_H

#include "define.h"
#include "netcode.h"
#include "network_client.h"

STRUCT_DEFINE(NetworkClientStandard)
{
    NetworkClient   base;
};

void        network_client_standard_init(R(UdpSocket*) sock, R(UdpClient*) udpClient, R(NetworkClientStandard*) client, uint32_t index);
void        network_client_standard_deinit(R(NetworkClientStandard*) client);

#define     network_client_standard_basic(cli) network_client_basic(&(cli)->base)

#define     network_client_standard_increment_packets_received(cli) network_client_increment_packets_received(&(cli)->base)

#endif//EQP_NETWORK_CLIENT_STANDARD_H
