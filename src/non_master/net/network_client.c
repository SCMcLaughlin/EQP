
#include "network_client.h"
#include "eqp_basic.h"
#include "udp_socket.h"
#include "udp_client.h"

void network_client_init(UdpSocket* sock, UdpClient* udpClient, NetworkClient* client, uint32_t index)
{
    Basic* basic                    = udp_socket_basic(sock);
    client->socketFd                = udp_socket_fd(sock);
    client->address.sin_addr.s_addr = udp_client_ip(udpClient);
    client->address.sin_port        = udp_client_port(udpClient);
    client->basic                   = basic;
    client->udpSocket               = sock;
    client->udpClientIndex          = index;
    client->packetsSent             = 0;
    client->packetsReceived         = 0;
    client->accountId               = 0;
}

void network_client_deinit(NetworkClient* client)
{
    (void)client;
}

int network_client_send_no_increment(NetworkClient* client, const void* data, uint32_t len)
{
    uint32_t i;
    printf("Send raw (%u):\n", len);
    for (i = 0; i < len; i++) printf("%02x ", ((byte*)data)[i]); printf("\n");
    return sendto(client->socketFd, (const char*)data, len, 0, (struct sockaddr*)&client->address, sizeof(IpAddress));
}

int network_client_send(NetworkClient* client, const void* data, uint32_t len)
{
    client->packetsSent++;
    return network_client_send_no_increment(client, data, len);
}

void network_client_flag_connection_as_dead(NetworkClient* client)
{
    udp_socket_flag_client_as_dead_by_index(client->udpSocket, client->udpClientIndex);
}
