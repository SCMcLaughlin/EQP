
#include "login_client.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"

LoginClient* login_client_create(R(ProtocolHandler*) handler, int expansion, int state)
{
    LoginClient* client = eqp_alloc_type(protocol_handler_basic(handler), LoginClient);
    
    client->state       = state;
    client->expansion   = expansion;
    client->handler     = handler;
    
    return client;
}

void login_client_set_password_temp(R(LoginClient*) client, R(const char*) password, int length)
{
    if ((uint32_t)length >= sizeof(client->passwordTemp))
        length = sizeof(client->passwordTemp) - 1;
    
    memcpy(client->passwordTemp, password, length);
    client->passwordLength = length;
}
