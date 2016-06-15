
#ifndef EQP_CHAR_SELECT_CLIENT_H
#define EQP_CHAR_SELECT_CLIENT_H

#include "define.h"
#include "expansion.h"
#include "protocol_handler.h"

STRUCT_DEFINE(CharSelectClient)
{
    bool                authorized;
    int                 expansion;
    ProtocolHandler*    handler;
};

CharSelectClient*   char_select_client_create(R(ProtocolHandler*) handler, int expansion);

#define             char_select_client_handler(client) ((client)->handler)

#endif//EQP_CHAR_SELECT_CLIENT_H
