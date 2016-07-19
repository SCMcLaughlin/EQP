
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
#include "item.h"
#include "item_prototype.h"
#include <zlib.h>

void    client_trilogy_schedule_packet_individual(Client* client, PacketTrilogy* packet);
void    client_trilogy_schedule_packet_broadcast(Client* client, PacketTrilogy* packet);
void    client_trilogy_send_zero_length_packet(Client* client, uint16_t opcode);
void    client_trilogy_send_zeroed_packet_var_length(Client* client, uint16_t opcode, uint32_t length);
void    client_trilogy_send_player_profile(Client* client);
void    client_trilogy_send_zone_entry(Client* client);
void    client_trilogy_send_keep_alive(ProtocolHandler* handler);

PacketTrilogy*  client_trilogy_make_op_weather(ZC* zc, int weatherType, int intensity);
PacketTrilogy*  client_trilogy_make_op_zone_info(ZC* zc, Zone* zone);
PacketTrilogy*  client_trilogy_make_op_spawn_appearance(ZC* zc, uint16_t entityId, uint16_t type, uint32_t value);
PacketTrilogy*  client_trilogy_make_op_spawn(ZC* zc, Mob* spawningMob);
PacketTrilogy*  client_trilogy_make_op_custom_message(ZC* zc, uint32_t chatChannel, const char* str, uint32_t len);
PacketTrilogy*  client_trilogy_make_op_custom_message_format(ZC* zc, uint32_t chatChannel, const char* fmt, va_list args);
PacketTrilogy*  client_trilogy_make_item_packet(ZC* zc, uint16_t opcode, Item* item, uint16_t slotId);

#endif//EQP_CLIENT_PACKET_TRILOGY_OUTPUT_H
