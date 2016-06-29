
#ifndef EQP_NETWORK_CLIENT_STANDARD_H
#define EQP_NETWORK_CLIENT_STANDARD_H

#include "define.h"
#include "netcode.h"
#include "network_client.h"

STRUCT_DEFINE(NetworkClientStandard)
{
    NetworkClient   base;
};

void        network_client_standard_init(UdpSocket* sock, UdpClient* udpClient, NetworkClientStandard* client, uint32_t index);
void        network_client_standard_deinit(NetworkClientStandard* client);

#define     network_client_standard_basic(cli) network_client_basic(&(cli)->base)
#define     network_client_standard_update_index(cli, i) network_client_update_udp_client_index(&(cli)->base, i)
#define     network_client_standard_increment_packets_received(cli) network_client_increment_packets_received(&(cli)->base)
#define     network_client_standard_flag_connection_as_dead(cli) network_client_flag_connection_as_dead(&(cli)->base)

#define     network_client_standard_send_immediate(cli, data, len) network_client_send(&(cli)->base, (data), (len))

#endif//EQP_NETWORK_CLIENT_STANDARD_H
