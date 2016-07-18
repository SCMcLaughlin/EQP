
#ifndef EQP_CHAR_SELECT_STRUCTS_TRILOGY_H
#define EQP_CHAR_SELECT_STRUCTS_TRILOGY_H

#include "define.h"
#include "player_profile.h"
#include "char_create_lua.h"

#define EQP_CHAR_SELECT_MAX_GUILDS          512
#define EQP_CHAR_SELECT_MAX_NAME_LENGTH     30
#define EQP_CHAR_SELECT_MAX_SURNAME_LENGTH  20

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
    uint8_t     weirdA[10][4];
    uint8_t     weirdB[10][4];
    uint8_t     unknownC[148];
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

STRUCT_DEFINE(CSTrilogy_NameApproval)
{
    char        name[32];
    uint32_t    race;
    uint32_t    class;
};

STRUCT_DEFINE(CSTrilogy_WearChange)
{
    uint32_t    unusedSpawnId;
    uint8_t     slot;
    uint8_t     material;   // Also used for IT# - Velious only goes up to 250
    uint16_t    operation;
    uint32_t    color;
    uint8_t     unknownA;
    uint8_t     flag;
    uint16_t    unknownB;
};

STRUCT_DEFINE(CSTrilogy_ZoneUnavailable)
{
    char    shortName[20];
};

STRUCT_DEFINE(CSTrilogy_ZoneAddress)
{
    char        ipAddress[75];
    char        zoneShortName[53];
    uint16_t    port;
};

STRUCT_DEFINE(CSTrilogy_TimeOfDay)
{
    uint8_t     hour;
    uint8_t     minute;
    uint8_t     day;
    uint8_t     month;
    uint16_t    year;
};

#pragma pack()

#endif//EQP_CHAR_SELECT_STRUCTS_TRILOGY_H
