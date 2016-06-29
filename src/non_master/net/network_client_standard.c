
#include "network_client_standard.h"

void network_client_standard_init(UdpSocket* sock, UdpClient* udpClient, NetworkClientStandard* client, uint32_t index)
{
    network_client_init(sock, udpClient, &client->base, index);
}

void network_client_standard_deinit(NetworkClientStandard* client)
{
    network_client_deinit(&client->base);
}
