
#include "client.h"
#include "zone.h"
#include "zone_cluster.h"

void* client_create_from_new_connection_standard(R(ProtocolHandler*) handler)
{
    return handler;
}

void* client_create_from_new_connection_trilogy(R(ProtocolHandler*) handler)
{
    //R(ZC*) zc = (ZC*)protocol_handler_basic(handler);
    
    //fixme: need intermediate, "unidentified client" type since we won't know which Client to match them up with for a few packets
    return handler;
}

void client_on_disconnect(R(void*) vclient, int isLinkdead)
{
    (void)vclient;
    printf("DISCONNECT (%s)\n", isLinkdead ? "explicit" : "timeout");
}

Client* client_create(R(ZC*) zc, R(Zone*) zone, R(Server_ClientZoning*) zoning)
{
    Query query;
    R(Database*) db = core_db(C(zc));
    Client* client  = eqp_alloc_type(B(zc), Client);
    
    memset(client, 0, sizeof(Client));
    
    atomic_init(&client->refCount, 1);
    
    mob_init_client(&client->mob, zc, zone);
    
    query_init(&query);
    
    return client;
}

void client_drop(R(Client*) client)
{
    if (atomic_fetch_sub(&client->refCount, 1) > 1)
        return;
    
    free(client);
}
