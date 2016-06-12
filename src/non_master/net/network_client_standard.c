
#include "network_client_standard.h"

void network_client_standard_init(R(UdpSocket*) sock, R(UdpClient*) udpClient, R(NetworkClientStandard*) client, uint32_t index)
{
    network_client_init(sock, udpClient, &client->base, index);
}

void network_client_standard_deinit(R(NetworkClientStandard*) client)
{
    network_client_deinit(&client->base);
}
