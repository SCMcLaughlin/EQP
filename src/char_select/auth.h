
#ifndef EQP_AUTH_H
#define EQP_AUTH_H

#include "define.h"

STRUCT_DEFINE(CharSelectAuth)
{
    uint32_t    accountId;
    bool        isLocal;
    uint64_t    timestamp;
    char        accountName[32];
    char        sessionKey[32];
};

#endif//EQP_AUTH_H
