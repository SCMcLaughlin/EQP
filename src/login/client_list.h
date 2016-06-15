
#ifndef EQP_CLIENT_LIST_H
#define EQP_CLIENT_LIST_H

#include "define.h"
#include "login_client.h"
#include "eqp_array.h"

STRUCT_DECLARE(Basic);

STRUCT_DEFINE(ClientByAccountId)
{
    uint32_t        accountId;
    LoginClient*    client;
};

STRUCT_DEFINE(ClientList)
{
    Array* array;
};

void            client_list_init(R(Basic*) basic, R(ClientList*) list);
void            client_list_deinit(R(ClientList*) list);

void            client_list_add(R(Basic*) basic, R(ClientList*) list, R(LoginClient*) client);
void            client_list_remove(R(ClientList*) list, R(LoginClient*) client);
LoginClient*    client_list_get(R(ClientList*) list, uint32_t accountId);

#endif//EQP_CLIENT_LIST_H
