
#ifndef EQP_CHAR_SELECT_CLIENT_TRILOGY_H
#define EQP_CHAR_SELECT_CLIENT_TRILOGY_H

#include "define.h"
#include "aligned.h"
#include "network_interface.h"
#include "protocol_handler.h"
#include "protocol_handler_trilogy.h"
#include "packet_trilogy.h"
#include "char_select_client.h"
#include "char_select_structs_trilogy.h"

/* Login handshake, in order of appearance */
#define TrilogyOp_LoginInfo         0x1858
#define TrilogyOp_LoginApproved     0x1007
#define TrilogyOp_Enter             0x8001
#define TrilogyOp_ExpansionInfo     0x21d8
#define TrilogyOp_CharacterInfo     0x2047
#define TrilogyOp_GuildList         0x2192

#endif//EQP_CHAR_SELECT_CLIENT_TRILOGY_H
