
#ifndef EQP_SERVER_STRUCTS_H
#define EQP_SERVER_STRUCTS_H

#include "define.h"

STRUCT_DEFINE(Server_ClientZoning)
{
    uint32_t    accountId;
    uint32_t    ipAddress;
    int64_t     characterId;
    bool        isLocal;
    bool        isFromCharSelect;
    char        accountName[64];
    char        characterName[32];
};

STRUCT_DEFINE(Server_ClientZoningReject)
{
    uint32_t    accountId;
    char        zoneShortName[32];
};

STRUCT_DEFINE(Server_ZoneAddress)
{
    uint32_t    accountId;
    char        ipAddress[128];
    uint16_t    portHostByteOrder;
    char        shortName[32];
    uint64_t    eqTimeBaseUnixSeconds;
    uint32_t    motdLength;
    char        messageOfTheDay[1];
};

#endif//EQP_SERVER_STRUCTS_H
