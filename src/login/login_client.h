
#ifndef EQP_LOGIN_CLIENT_H
#define EQP_LOGIN_CLIENT_H

#include "define.h"
#include "protocol_handler.h"

STRUCT_DEFINE(LoginClient)
{
    int                 state;
    int                 expansion;
    ProtocolHandler*    handler;
    char                passwordTemp[32];
    int                 passwordLength;
};

LoginClient*    login_client_create(R(ProtocolHandler*) handler, int expansion, int state);

#define         login_client_get_state(client) ((client)->state)
#define         login_client_set_state(client, st) ((client)->state = (st))
#define         login_client_handler(client) ((client)->handler)
#define         login_client_password_temp(client) ((client)->passwordTemp)
#define         login_client_password_length(client) ((client)->passwordLength)

void            login_client_set_password_temp(R(LoginClient*) client, R(const char*) password, int length);
#define         login_client_clear_password_temp(client) memset((client)->passwordTemp, 0, sizeof((client)->passwordTemp))

#endif//EQP_LOGIN_CLIENT_H
