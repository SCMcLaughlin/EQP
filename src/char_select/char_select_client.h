
#ifndef EQP_CHAR_SELECT_CLIENT_H
#define EQP_CHAR_SELECT_CLIENT_H

#include "define.h"
#include "expansion.h"
#include "protocol_handler.h"
#include "auth.h"

STRUCT_DEFINE(CharSelectClient)
{
    int                 expansion;
    ProtocolHandler*    handler;
    CharSelectAuth      auth;
};

CharSelectClient*   char_select_client_create(R(ProtocolHandler*) handler, int expansion);

void                char_select_client_set_auth(R(CharSelectClient*) client, R(CharSelectAuth*) auth);

#define             char_select_client_handler(client) ((client)->handler)
#define             char_select_client_is_authed(client) ((client)->auth.accountId != 0)
#define             char_select_client_account_id(client) ((client)->auth.accountId)

#endif//EQP_CHAR_SELECT_CLIENT_H
