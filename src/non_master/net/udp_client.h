
#ifndef EQP_UDP_CLIENT_H
#define EQP_UDP_CLIENT_H

#include "define.h"

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(ProtocolHandler);

STRUCT_DEFINE(UdpClient)
{
    uint32_t ip;
    uint16_t port;
    union
    {
        struct
        {
            uint16_t isDead : 1;
            uint16_t hasInputPacketsQueued : 1;
            uint16_t hasOutputPacketsQueued : 1;
            uint16_t unused : 13;
        };
        
        uint16_t flags;
    };
    ProtocolHandler* handler;
};

void    udp_client_init(R(Basic*) basic, R(UdpClient*) client, uint32_t ip, uint16_t port);
void    udp_client_deinit(R(UdpClient*) client);

#define udp_client_ip(cli) ((cli)->ip)
#define udp_client_port(cli) ((cli)->port)
#define udp_client_handler(cli) ((cli)->handler)

#define udp_client_is_dead(cli) ((cli)->isDead)

#endif//EQP_UDP_CLIENT_H
