
#ifndef EQP_UDP_CLIENT_H
#define EQP_UDP_CLIENT_H

#include "define.h"
#include "eqp_clock.h"

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
            uint16_t isDead             : 1;
            uint16_t ignoreAllPackets   : 1;
            uint16_t unused             : 14;
        };
        
        uint16_t flags;
    };
    ProtocolHandler*    handler;
    uint64_t            lastRecvTime;
};

void    udp_client_init(Basic* basic, UdpClient* client, uint32_t ip, uint16_t port);
void    udp_client_deinit(UdpClient* client);

void    udp_client_update_index(UdpClient* client, uint32_t index);

#define udp_client_ip(cli) ((cli)->ip)
#define udp_client_port(cli) ((cli)->port)
#define udp_client_handler(cli) ((cli)->handler)

#define udp_client_is_dead(cli) ((cli)->isDead)
#define udp_client_flag_as_dead(cli) ((cli)->isDead = true)
#define udp_client_ignore_all_packets(cli) ((cli)->ignoreAllPackets)
#define udp_client_flag_to_ignore_all_packets(cli) ((cli)->ignoreAllPackets = true)

#define udp_client_last_recv_time(cli) ((cli)->lastRecvTime)
#define udp_client_update_last_recv_time(cli) ((cli)->lastRecvTime = clock_milliseconds())

#endif//EQP_UDP_CLIENT_H
