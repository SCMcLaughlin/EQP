
#ifndef EQP_PLAYER_PROFILE_H
#define EQP_PLAYER_PROFILE_H

#include "define.h"
#include "enums.h"
#include "skills.h"
#include "defines_trilogy.h"
#include "structs.h"

#pragma pack(1)

STRUCT_DEFINE(Trilogy_Buff)
{
    uint8_t     visibility; // 0 = not visible, 1 = permanent, 2 = normal
    uint8_t     casterLevel;
    uint8_t     bardModifier;
    uint8_t     unknown;
    uint16_t    spellId;
    uint32_t    tickDuration;
};

STRUCT_DEFINE(Trilogy_PPItem)
{
    uint16_t    unknownA;
    uint8_t     charges;    // 0xff = unlimited
    uint8_t     unknownB;   // maybe charges is 16bit?
    uint16_t    unknownDefaultFF;
    uint8_t     unknownC[4];
};

STRUCT_DEFINE(Trilogy_PPGroupMember)
{
    char    name[16];
    uint8_t unknown[32];
};

#define TRILOGY_PP_FIELDS                                                                                               \
    char                    name[30];                                                                                   \
    char                    surname[20];                                                                                \
    uint16_t                gender;                                                                                     \
    uint16_t                race;                                                                                       \
    uint16_t                class;                                                                                      \
    uint32_t                level;                                                                                      \
    uint32_t                experience;                                                                                 \
    uint16_t                trainingPoints;                                                                             \
    int16_t                 currentMana;                                                                                \
    uint8_t                 face;                                                                                       \
    uint8_t                 unknownA[47];                                                                               \
    int16_t                 currentHp;                                                                                  \
    uint8_t                 unknownB;                                                                                   \
    uint8_t                 STR;                                                                                        \
    uint8_t                 STA;                                                                                        \
    uint8_t                 CHA;                                                                                        \
    uint8_t                 DEX;                                                                                        \
    uint8_t                 INT;                                                                                        \
    uint8_t                 AGI;                                                                                        \
    uint8_t                 WIS;                                                                                        \
    uint8_t                 languages[EQP_TRILOGY_LANGUAGES_COUNT];                                                     \
    uint8_t                 unknownC[14];                                                                               \
    uint16_t                mainInventoryItemIds[30];                                                                   \
    uint32_t                mainInventoryInternalUnused[30];                                                            \
    Trilogy_PPItem          mainInventoryItemProperties[30];                                                            \
    Trilogy_Buff            buffs[EQP_TRILOGY_MAX_BUFFS];                                                               \
    uint16_t                baggedItemIds[90];  /* [80] to [89] are cursor bag slots */                                 \
    Trilogy_PPItem          baggedItemProperties[90];                                                                   \
    uint16_t                spellbook[EQP_TRILOGY_SPELLBOOK_SLOT_COUNT];                                                \
    uint16_t                memmedSpellIds[EQP_TRILOGY_MEMMED_SPELL_SLOT_COUNT];                                        \
    uint16_t                unknownD;                                                                                   \
    float                   y; /*fixme: confirm this*/                                                                  \
    float                   x;                                                                                          \
    float                   z;                                                                                          \
    float                   heading;                                                                                    \
    char                    zoneShortName[32];                                                                          \
    uint32_t                unknownEDefault100;   /* Breath or something? */                                            \
    Coin                    coins;                                                                                      \
    Coin                    coinsBank;                                                                                  \
    Coin                    coinsCursor;                                                                                \
    uint8_t                 skills[EQP_TRILOGY_SKILLS_COUNT];                                                           \
    uint8_t                 unknownF[162];                                                                              \
    uint32_t                autoSplit;  /* 0 or 1 */                                                                    \
    uint32_t                pvpEnabled;                                                                                 \
    uint8_t                 unknownG[12]; /*starts with uint32_t isAfk?*/                                               \
    uint32_t                isGM;                                                                                       \
    uint8_t                 unknownH[20];                                                                               \
    uint32_t                disciplinesReady;                                                                           \
    uint8_t                 unknownI[20];                                                                               \
    uint32_t                hunger;                                                                                     \
    uint32_t                thirst;                                                                                     \
    uint8_t                 unknownJ[24];                                                                               \
    char                    bindZoneShortName[5][20];                                                                   \
    Trilogy_PPItem          bankInventoryItemProperties[8];                                                             \
    Trilogy_PPItem          bankBaggedItemProperties[80];                                                               \
    uint32_t                unknownK;                                                                                   \
    float                   bindLocY[5];                                                                                \
    float                   bindLocX[5];                                                                                \
    float                   bindLocZ[5];                                                                                \
    float                   bindLocHeading[5];                                                                          \
    uint8_t                 unknownL[4];                                                                                \
    uint32_t                bankInventoryInternalUnused[8];                                                             \
    uint8_t                 unknownM[12];                                                                               \
    uint32_t                unixTimeA;  /*creation time?*/                                                              \
    uint8_t                 unknownN[8];                                                                                \
    uint32_t                unknownODefault1;                                                                           \
    uint8_t                 unknownP[8];                                                                                \
    uint16_t                bankInventoryItemIds[8];                                                                    \
    uint16_t                bankBaggedItemIds[80];                                                                      \
    uint16_t                deity;                                                                                      \
    uint16_t                guildId;                                                                                    \
    uint32_t                unixTimeB;                                                                                  \
    uint8_t                 unknownQ[4];                                                                                \
    uint16_t                unknownRDefault7f7f;                                                                        \
    uint8_t                 fatiguePercent; /* Percentage taken out of the yellow bar, i.e. (100 - fatiguePercent) */   \
    uint8_t                 unknownS;                                                                                   \
    uint8_t                 unknownTDefault1;                                                                           \
    uint16_t                anon;                                                                                       \
    uint8_t                 guildRank;                                                                                  \
    uint8_t                 drunkeness;                                                                                 \
    uint8_t                 showEqLoadScreen;                                                                           \
    uint16_t                unknownU;                                                                                   \
    uint32_t                spellGemRefreshMilliseconds[EQP_TRILOGY_MEMMED_SPELL_SLOT_COUNT];                           \
    uint32_t                unknownV; /* Is the above field supposed to be 4 bytes lower? */                            \
    uint32_t                harmtouchRefreshMilliseconds;                                                               \
    Trilogy_PPGroupMember   groupMember[5];                                                                             \
    uint8_t                 unknownW[3644]

STRUCT_DEFINE(Trilogy_PlayerProfile)
{
    uint32_t    crc;    // crc_calc32, negated
    TRILOGY_PP_FIELDS;
};

STRUCT_DEFINE(Trilogy_PlayerProfile_CharCreation)
{
    TRILOGY_PP_FIELDS;
};

#pragma pack()

#undef TRILOGY_PP_FIELDS

#endif//EQP_PLAYER_PROFILE_H
