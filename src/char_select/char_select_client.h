
#ifndef EQP_CHAR_SELECT_CLIENT_H
#define EQP_CHAR_SELECT_CLIENT_H

#include "define.h"
#include "expansion.h"
#include "protocol_handler.h"
#include "auth.h"

STRUCT_DECLARE(CharSelect);

STRUCT_DEFINE(CharSelectClient)
{
    int                 expansion;
    ProtocolHandler*    handler;
    CharSelectAuth      auth;
    uint32_t            weaponMaterialsTrilogy[10][2];
};

CharSelectClient*   char_select_client_create(R(ProtocolHandler*) handler, int expansion);

void                char_select_client_set_auth(R(CharSelectClient*) client, R(CharSelectAuth*) auth);
void                char_select_client_query_account_id(R(CharSelectClient*) client, R(CharSelect*) charSelect);

#define             char_select_client_handler(client) ((client)->handler)
#define             char_select_client_is_authed(client) ((client)->auth.accountId != 0)
#define             char_select_client_account_id(client) ((client)->auth.accountId)

#endif//EQP_CHAR_SELECT_CLIENT_H
