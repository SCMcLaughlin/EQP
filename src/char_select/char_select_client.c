
#include "char_select_client.h"

CharSelectClient* char_select_client_create(R(ProtocolHandler*) handler, int expansion)
{
    CharSelectClient* client = eqp_alloc_type(protocol_handler_basic(handler), CharSelectClient);
    
    client->authorized  = false;
    client->expansion   = expansion;
    client->handler     = handler;
    
    return client;
}

void client_on_disconnect(R(void*) vclient, int isLinkdead)
{
    R(CharSelectClient*) client = (CharSelectClient*)vclient;
    
    if (client)
    {
        free(client);
    }
    
    printf("DISCONNECTED (%s)\n", isLinkdead ? "timeout" : "explicit");
}
