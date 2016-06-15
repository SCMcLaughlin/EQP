
#ifndef EQP_LOGIN_CLIENT_H
#define EQP_LOGIN_CLIENT_H

#include "define.h"
#include "expansion.h"
#include "random.h"
#include "protocol_handler.h"
#include "eqp_string.h"

#define LOGIN_RESPONSE_ACCEPTED     1
#define LOGIN_RESPONSE_DENIED       0
#define LOGIN_RESPONSE_SUSPENDED    -1
#define LOGIN_RESPONSE_BANNED       -2
#define LOGIN_RESPONSE_WORLD_FULL   -3

STRUCT_DEFINE(LoginClient)
{
    int                 state;
    int                 expansion;
    ProtocolHandler*    handler;
    String*             accountName;
    uint32_t            accountId;
    char                passwordTemp[32];
    int                 passwordLength;
    char                sessionKey[16];
};

LoginClient*    login_client_create(R(ProtocolHandler*) handler, int expansion, int state);

#define         login_client_get_state(client) ((client)->state)
#define         login_client_set_state(client, st) ((client)->state = (st))
#define         login_client_handler(client) ((client)->handler)
#define         login_client_account_name(client) ((client)->accountName)
#define         login_client_account_id(client) ((client)->accountId)
#define         login_client_password_temp(client) ((client)->passwordTemp)
#define         login_client_password_length(client) ((client)->passwordLength)
#define         login_client_session_key(client) ((client)->sessionKey)

void            login_client_set_account_name(R(LoginClient*) client, R(const char*) name, int length);
#define         login_client_set_account_id(client, id) ((client)->accountId = (id))
void            login_client_set_password_temp(R(LoginClient*) client, R(const char*) password, int length);
#define         login_client_clear_password_temp(client) memset((client)->passwordTemp, 0, sizeof((client)->passwordTemp))

void            login_client_handle_login_response(R(LoginClient*) client, int response);

void            login_client_generate_session_key(R(LoginClient*) client);

#endif//EQP_LOGIN_CLIENT_H
