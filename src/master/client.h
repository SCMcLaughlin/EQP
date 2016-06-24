
#ifndef EQP_CLIENT_H
#define EQP_CLIENT_H

#include "define.h"

STRUCT_DEFINE(Client)
{
    int64_t     characterId;
    uint32_t    accountId;
    char        name[32];
};

#endif//EQP_CLIENT_H
