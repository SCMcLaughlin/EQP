
#include "login_client.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"
#include "client_list.h"
#include "eqp_login.h"
#include "login_client_trilogy.h"

LoginClient* login_client_create(R(ProtocolHandler*) handler, int expansion, int state)
{
    LoginClient* client = eqp_alloc_type(protocol_handler_basic(handler), LoginClient);
    
    client->state       = state;
    client->expansion   = expansion;
    client->handler     = handler;
    client->accountName = NULL;
    client->accountId   = 0;
    
    return client;
}

void client_on_disconnect(R(void*) vclient, int isLinkdead)
{
    R(LoginClient*) client = (LoginClient*)vclient;
    
    if (client)
    {
        R(ClientList*) clientList = login_client_list((Login*)protocol_handler_basic(client->handler));
        
        client_list_remove(clientList, client);
        
        if (client->accountName)
            string_destroy(client->accountName);
        
        free(client);
    }
    
    printf("DISCONNECTED (%s)\n", isLinkdead ? "timeout" : "explicit");
}

void login_client_set_account_name(R(LoginClient*) client, R(const char*) name, int length)
{
    client->accountName = string_create_from_cstr(protocol_handler_basic(client->handler), name, length);
}

void login_client_set_password_temp(R(LoginClient*) client, R(const char*) password, int length)
{
    if ((uint32_t)length >= sizeof(client->passwordTemp))
        length = sizeof(client->passwordTemp) - 1;
    
    memcpy(client->passwordTemp, password, length);
    client->passwordLength = length;
}

void login_client_handle_login_response(R(LoginClient*) client, int response)
{
    if (client->expansion == ExpansionId_Trilogy)
        login_client_trilogy_handle_login_response(client, response);
    //else
    //  login_client_standard_handle_login_response(client, response);
}

void login_client_generate_session_key(R(LoginClient*) client)
{
    R(char*) key    = client->sessionKey;
    uint32_t n      = sizeof(client->sessionKey);
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        static const char keys[] =
        {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
            'Y', 'Z', '0', '1', '2', '3', '4', '5',
            '6', '7', '8', '9'
        };
        
        key[i] = keys[random_uint32() % sizeof(keys)];
    }
    
    key[n] = 0;
}
