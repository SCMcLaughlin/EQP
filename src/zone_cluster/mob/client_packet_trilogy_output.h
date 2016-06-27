
#ifndef EQP_CLIENT_PACKET_TRILOGY_OUTPUT_H
#define EQP_CLIENT_PACKET_TRILOGY_OUTPUT_H

#include "define.h"
#include "network_interface.h"
#include "client.h"
#include "opcodes_trilogy.h"
#include "structs_trilogy.h"
#include "packet_trilogy.h"
#include "crc.h"
#include "eqp_clock.h"
#include "zone_id.h"
#include <zlib.h>

void    client_trilogy_schedule_packet_individual(R(Client*) client, R(PacketTrilogy*) packet);
void    client_trilogy_schedule_packet_broadcast(R(Client*) client, R(PacketTrilogy*) packet);
void    client_trilogy_send_player_profile(R(Client*) client);
void    client_trilogy_send_zone_entry(R(Client*) client);

PacketTrilogy*  client_trilogy_make_op_weather(R(ZC*) zc, int weatherType, int intensity);
PacketTrilogy*  client_trilogy_make_op_zone_info(R(ZC*) zc, R(Zone*) zone);

#endif//EQP_CLIENT_PACKET_TRILOGY_OUTPUT_H
