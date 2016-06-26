
#ifndef EQP_CLIENT_H
#define EQP_CLIENT_H

#include "define.h"
#include "zone_cluster.h"
#include "source_id.h"
#include "server_structs.h"

STRUCT_DECLARE(ClientMgr);

STRUCT_DEFINE(Client)
{
    int64_t         characterId;
    uint32_t        accountId;
    uint32_t        ipAddress;
    uint16_t        zoneId;
    uint16_t        instanceId;
    bool            isLocal;
    ZoneCluster*    zoneCluster;
    ClientMgr*      clientMgr;
    char            name[32];
    char            accountName[64];
};

STRUCT_DEFINE(ClientByIds)
{
    int64_t     characterId;
    uint32_t    accountId;
    uint32_t    ipAddress;
    Client*     client;
};

Client* client_create(R(Basic*) basic, R(ClientMgr*) mgr, R(Server_ClientZoning*) zoning);
#define client_destroy(cli) free(cli)

#define client_set_character_id(cli, id) ((cli)->characterId = (id))
#define client_character_id(cli) ((cli)->characterId)
#define client_account_id(cli) ((cli)->accountId)
#define client_ip_address(cli) ((cli)->ipAddress)
#define client_client_mgr(cli) ((cli)->clientMgr)
#define client_set_zone_id(cli, id) ((cli)->zoneId = (id))
#define client_zone_id(cli) ((cli)->zoneId)
#define client_set_instance_id(cli, id) ((cli)->instanceId = (id))
#define client_instance_id(cli) ((cli)->instanceId)
#define client_is_local(cli) ((cli)->isLocal)
#define client_set_zone_cluster(cli, zc) ((cli)->zoneCluster = (zc))
#define client_zone_cluster(cli) ((cli)->zoneCluster)
#define client_name(cli) ((cli)->name)
#define client_account_name(cli) ((cli)->accountName)
#define client_zone_source_id(cli) ((cli)->zoneId + ((cli)->instanceId * EQP_SOURCE_ID_ZONE_INSTANCE_OFFSET))

#endif//EQP_CLIENT_H
