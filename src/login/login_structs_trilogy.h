
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

STRUCT_DEFINE(LoginTrilogy_ServerListHeader)
{
    uint8_t     serverCount;
    uint8_t     unknown[2];
    uint8_t     showNumPlayers; // 0xff to show the count, 0 to show "UP"
};

STRUCT_DEFINE(LoginTrilogy_ServerFooter)
{
    uint8_t     isGreenName;
    int         playerCount; // -1 for "DOWN", -2 for "LOCKED"
};

STRUCT_DEFINE(LoginTrilogy_ServerListFooter)
{
    uint32_t    admin;
    uint8_t     zeroesA[8];
    uint8_t     kunark;
    uint8_t     velious;
    uint8_t     zeroesB[12];
};

#pragma pack()

#endif//EQP_LOGIN_STRUCTS_TRILOGY_H
