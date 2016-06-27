
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

void    client_trilogy_send_player_profile(R(Client*) client);

#endif//EQP_CLIENT_PACKET_TRILOGY_OUTPUT_H
