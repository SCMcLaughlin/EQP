
#include "client.h"
#include "client_manager.h"

Client* client_create(Basic* basic, ClientMgr* mgr, Server_ClientZoning* zoning)
{
    Client* client = eqp_alloc_type(basic, Client);
    
    client->characterId = 0;
    client->accountId   = zoning->accountId;
    client->ipAddress   = zoning->ipAddress;
    client->zoneId      = 0;
    client->instanceId  = 0;
    client->isLocal     = zoning->isLocal;
    client->zoneCluster = NULL;
    client->clientMgr   = mgr;
    snprintf(client->name, sizeof(client->name), "%s", zoning->characterName);
    snprintf(client->accountName, sizeof(client->accountName), "%s", zoning->accountName);
    
    return client;
}
