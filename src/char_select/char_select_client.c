
#include "char_select_client.h"
#include "char_select_client_trilogy.h"
#include "eqp_char_select.h"

CharSelectClient* char_select_client_create(R(ProtocolHandler*) handler, int expansion)
{
    CharSelectClient* client = eqp_alloc_type(protocol_handler_basic(handler), CharSelectClient);
    
    memset(client, 0, sizeof(CharSelectClient));
    
    client->expansion   = expansion;
    client->handler     = handler;
    
    return client;
}

void client_on_disconnect(R(void*) vclient, int isLinkdead)
{
    R(CharSelectClient*) client = (CharSelectClient*)vclient;
    
    if (client)
    {
        char_select_remove_client_from_unauthed_list((CharSelect*)protocol_handler_basic(client->handler), client);
        
        free(client);
    }
    
    printf("DISCONNECTED (%s)\n", isLinkdead ? "timeout" : "explicit");
}

void char_select_client_set_auth(R(CharSelectClient*) client, R(CharSelectAuth*) auth)
{
    client->auth = *auth;
    
    if (client->expansion == ExpansionId_Trilogy)
        cs_client_trilogy_on_auth(client);
    //else
    //    cs_client_standard_on_auth(client);
}
