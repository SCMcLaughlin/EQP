
#include "network_client.h"
#include "eqp_basic.h"
#include "udp_socket.h"
#include "udp_client.h"

void network_client_init(R(UdpSocket*) sock, R(UdpClient*) udpClient, R(NetworkClient*) client)
{
    R(Basic*) basic                 = udp_socket_basic(sock);
    client->socketFd                = udp_socket_fd(sock);
    client->address.sin_addr.s_addr = udp_client_ip(udpClient);
    client->address.sin_port        = udp_client_port(udpClient);
    client->basic                   = basic;
    client->packetsSent             = 0;
    client->packetsReceived         = 0;
    client->accountId               = 0;
    //client->inputPackets            = array_create_type(basic, PacketInput*);
}

void network_client_deinit(R(NetworkClient*) client)
{
    (void)client;
    /*if (client->inputPackets)
    {
        array_destroy(client->inputPackets);
        client->inputPackets = NULL;
    }*/
}

int network_client_send_no_increment(R(NetworkClient*) client, R(const void*) data, uint32_t len)
{
    uint32_t i;
    printf("Send raw:\n");
    for (i = 0; i < len; i++) printf("%02x ", ((byte*)data)[i]); printf("\n");
    return sendto(client->socketFd, (const char*)data, len, 0, (struct sockaddr*)&client->address, sizeof(IpAddress));
}

int network_client_send(R(NetworkClient*) client, R(const void*) data, uint32_t len)
{
    client->packetsSent++;
    return network_client_send_no_increment(client, data, len);
}
