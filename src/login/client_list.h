
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

void            client_list_init(Basic* basic, ClientList* list);
void            client_list_deinit(ClientList* list);

void            client_list_add(Basic* basic, ClientList* list, LoginClient* client);
void            client_list_remove(ClientList* list, LoginClient* client);
LoginClient*    client_list_get(ClientList* list, uint32_t accountId);

#endif//EQP_CLIENT_LIST_H
