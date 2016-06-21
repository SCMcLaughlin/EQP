
#ifndef EQP_CHAR_SELECT_STRUCTS_TRILOGY_H
#define EQP_CHAR_SELECT_STRUCTS_TRILOGY_H

#include "define.h"

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

STRUCT_DEFINE(CSTrilogy_CreateCharacter)
{
    char        name[EQP_CHAR_SELECT_MAX_NAME_LENGTH];
    char        surname[EQP_CHAR_SELECT_MAX_SURNAME_LENGTH];
    uint16_t    gender;
    uint16_t    race;
    uint16_t    class;
    uint32_t    level;
    uint32_t    experience;
    uint16_t    trainingPoints;
    int16_t     currentMana;
    uint8_t     face;
    uint8_t     unknownA[47];
    int16_t     currentHp;
    uint8_t     unknownB;
    union
    {
        struct
        {
            uint8_t STR;
            uint8_t STA;
            uint8_t CHA;
            uint8_t DEX;
            uint8_t INT;
            uint8_t AGI;
            uint8_t WIS;
        };
        
        uint8_t stats[7];
    };
    uint8_t     stuffCharSelectDoesntCareAboutA[4026];
    uint16_t    deity;
    uint8_t     stuffCharSelectDoesntCareAboutB[3946];
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

#pragma pack()

#endif//EQP_CHAR_SELECT_STRUCTS_TRILOGY_H
