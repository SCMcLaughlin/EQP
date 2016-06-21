
#ifndef EQP_CHAR_SELECT_CLIENT_H
#define EQP_CHAR_SELECT_CLIENT_H

#include "define.h"
#include "expansion.h"
#include "protocol_handler.h"
#include "auth.h"
#include <stdatomic.h>

STRUCT_DECLARE(CharSelect);

STRUCT_DEFINE(CharSelectClient)
{
    int                 expansion;
    ProtocolHandler*    handler;
    atomic_int          refCount;
    CharSelectAuth      auth;
    uint8_t             weaponMaterialsTrilogy[10][2];
    bool                isNameApproved;
};

CharSelectClient*   char_select_client_create(R(ProtocolHandler*) handler, int expansion);
#define             char_select_client_grab(cli) atomic_fetch_add(&(cli)->refCount, 1)
void                char_select_client_drop(R(CharSelectClient*) client);

void                char_select_client_set_auth(R(CharSelectClient*) client, R(CharSelectAuth*) auth);
void                char_select_client_query_account_id(R(CharSelectClient*) client, R(CharSelect*) charSelect);
void                char_select_client_query_character_name_taken(R(CharSelectClient*) client, R(CharSelect*) charSelect, R(const char*) name);
void                char_select_client_delete_character_by_name(R(CharSelectClient*) client, R(CharSelect*) charSelect, R(const char*) name);

#define             char_select_client_handler(client) ((client)->handler)
#define             char_select_client_is_authed(client) ((client)->auth.timestamp != 0)
#define             char_select_client_account_id(client) ((client)->auth.accountId)
#define             char_select_client_weapon_material(client, index, slot) ((client)->weaponMaterialsTrilogy[(index)][(slot) - 7])
#define             char_select_client_is_name_approved(client) ((client)->isNameApproved)
#define             char_select_client_set_name_approved(client, val) ((client)->isNameApproved = (val))

#endif//EQP_CHAR_SELECT_CLIENT_H
