
#ifndef EQP_SERVER_STRUCTS_H
#define EQP_SERVER_STRUCTS_H

#include "define.h"

STRUCT_DEFINE(Server_ClientZoning)
{
    uint32_t    accountId;
    uint32_t    ipAddress;
    int64_t     characterId;
    char        accountName[64];
    char        characterName[64];
};

#endif//EQP_SERVER_STRUCTS_H
