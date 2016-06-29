
#ifndef EQP_AUTH_H
#define EQP_AUTH_H

#include "define.h"

STRUCT_DECLARE(CharSelect);

STRUCT_DEFINE(CharSelectAuth)
{
    uint32_t    accountId;
    bool        isLocal;
    uint64_t    timestamp;
    int         accountStatus;
    int         isLocked;
    char        accountName[32];
    char        sessionKey[32];
    CharSelect* charSelect;
};

#endif//EQP_AUTH_H
