
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
#include "zone_id.h"

/* Login handshake, in order of appearance */
#define TrilogyOp_LoginInfo         0x1858
#define TrilogyOp_LoginApproved     0x1007
#define TrilogyOp_Enter             0x8001
#define TrilogyOp_ExpansionInfo     0x21d8
#define TrilogyOp_CharacterInfo     0x2047
#define TrilogyOp_GuildList         0x2192

/* Requests from the client */
#define TrilogyOp_NameApproval      0x208b
#define TrilogyOp_CreateCharacter   0x2049
#define TrilogyOp_DeleteCharacter   0x205a
#define TrilogyOp_WearChange        0x2092

void    cs_client_trilogy_on_auth(R(CharSelectClient*) client);
void    cs_client_trilogy_on_account_id(R(CharSelectClient*) client, uint32_t accountId);
void    cs_client_trilogy_on_character_name_checked(R(CharSelectClient*) client, int taken);

#endif//EQP_CHAR_SELECT_CLIENT_TRILOGY_H
