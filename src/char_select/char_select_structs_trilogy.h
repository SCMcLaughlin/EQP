
#ifndef EQP_CHAR_SELECT_STRUCTS_TRILOGY_H
#define EQP_CHAR_SELECT_STRUCTS_TRILOGY_H

#include "define.h"

#define EQP_CHAR_SELECT_MAX_GUILDS 512

#pragma pack(1)

STRUCT_DEFINE(CSTrilogy_CharSelectInfo)
{
    char        names[10][30];
    uint8_t     levels[10];
    uint8_t     classes[10];
    int16_t     races[10];
    char        zoneShortNames[10][20];
    uint8_t     genders[10];
    uint8_t     faces[10];
    uint8_t     materials[10][9];
    uint8_t     unknownA[2];
    uint32_t    materialTints[10][9];
    uint8_t     unknownB[20];
    uint8_t     unknownC[4];
    uint8_t     weirdA[8][4];
    uint8_t     unknownD[8];
    uint8_t     weirdB[8][4];
    uint8_t     unknownE[152];
};

STRUCT_DEFINE(CSTrilogy_GuildEntry)
{
    uint32_t    guildId;    // If not exists, 0xFFFFFFFF
    char        name[32];
    uint32_t    unknownA;   // 0xFFFFFFFF
    uint32_t    exists;
    uint32_t    unknownB;
    uint32_t    unknownC;   // 0xFFFFFFFF
    uint32_t    unknownD;
};

STRUCT_DEFINE(CSTrilogy_GuildList)
{
    uint32_t                unknown;
    CSTrilogy_GuildEntry    guilds[EQP_CHAR_SELECT_MAX_GUILDS];
};

#pragma pack()

#endif//EQP_CHAR_SELECT_STRUCTS_TRILOGY_H
