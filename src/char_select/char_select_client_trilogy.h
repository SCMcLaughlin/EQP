
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

STRUCT_DEFINE(CSTrilogy_CharCreateParams)
{
    uint32_t    accountId;
    char        name[32];
    uint8_t     gender;
    uint8_t     race;
    uint8_t     class;
    uint8_t     face;
    uint8_t     currentHp;
    union
    {
        struct
        {
            uint8_t     STR;
            uint8_t     STA;
            uint8_t     CHA;
            uint8_t     DEX;
            uint8_t     INT;
            uint8_t     AGI;
            uint8_t     WIS;
        };
        
        uint8_t stats[7];
    };
    uint8_t     deity;
    int         zoneId;
    float       x;
    float       y;
    float       z;
};

void    cs_client_trilogy_on_auth(R(CharSelectClient*) client);
void    cs_client_trilogy_on_account_id(R(CharSelectClient*) client, uint32_t accountId);
void    cs_client_trilogy_on_character_name_checked(R(CharSelectClient*) client, int taken);

#endif//EQP_CHAR_SELECT_CLIENT_TRILOGY_H
