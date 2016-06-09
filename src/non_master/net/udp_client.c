
#include "udp_client.h"
#include "eqp_basic.h"
#include "protocol_handler.h"

void udp_client_init(R(Basic*) basic, R(UdpClient*) client, uint32_t ip, uint16_t port)
{
    client->ip      = ip;
    client->port    = port;
    client->flags   = 0;
    client->handler = protocol_handler_create(basic);
}

void udp_client_deinit(R(UdpClient*) client)
{
    if (client->handler)
    {
        protocol_handler_destroy(client->handler);
        client->handler = NULL;
    }
}
