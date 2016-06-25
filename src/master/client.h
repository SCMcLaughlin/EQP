
#ifndef EQP_CLIENT_H
#define EQP_CLIENT_H

#include "define.h"

STRUCT_DEFINE(Client)
{
    int64_t     characterId;
    uint32_t    accountId;
    uint32_t    ipAddress;
    uint16_t    zoneId;
    uint16_t    instanceId;
    //ZoneCluster*  zoneCluster;
    char        name[32];
    char        accountName[64];
};

STRUCT_DEFINE(ClientByIds)
{
    int64_t     characterId;
    uint32_t    accountId;
    uint32_t    ipAddress;
    Client*     client;
};

#endif//EQP_CLIENT_H
