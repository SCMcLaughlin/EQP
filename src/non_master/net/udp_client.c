
#include "udp_client.h"
#include "eqp_basic.h"
#include "protocol_handler.h"

void udp_client_init(Basic* basic, UdpClient* client, uint32_t ip, uint16_t port)
{
    client->ip      = ip;
    client->port    = port;
    client->flags   = 0;
    client->handler = protocol_handler_create(basic);
}

void udp_client_deinit(UdpClient* client)
{
    if (client->handler)
    {
        protocol_handler_drop(client->handler);
        client->handler = NULL;
    }
}

void udp_client_update_index(UdpClient* client, uint32_t index)
{
    protocol_handler_update_index(client->handler, index);
}
