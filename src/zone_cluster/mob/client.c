
#include "client.h"

void* client_create_from_new_connection_standard(R(ProtocolHandler*) handler)
{
    return handler;
}

void* client_create_from_new_connection_trilogy(R(ProtocolHandler*) handler)
{
    return handler;
}

void client_on_disconnect(R(void*) client, int isLinkdead)
{
    
}
