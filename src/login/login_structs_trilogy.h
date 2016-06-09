
#ifndef EQP_LOGIN_STRUCTS_TRILOGY_H
#define EQP_LOGIN_STRUCTS_TRILOGY_H

#include "define.h"

#pragma pack(1)

STRUCT_DEFINE(LoginTrilogy_Credentials)
{
    char username[20];
    char password[20];
};

STRUCT_DEFINE(LoginTrilogy_Session)
{
    char        sessionId[10];
    char        unused[7];
    uint32_t    unknown;
};

#pragma pack()

#endif//EQP_LOGIN_STRUCTS_TRILOGY_H
