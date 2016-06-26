
#ifndef EQP_STRUCTS_TRILOGY_H
#define EQP_STRUCTS_TRILOGY_H

#include "define.h"
#include "enums.h"
#include "skills.h"
#include "defines_trilogy.h"
#include "structs.h"

#pragma pack(1)

STRUCT_DEFINE(Trilogy_ZoneEntry)
{
    uint32_t    checksum;
    char        characterName[32];
};

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
    uint8_t     charges; // 0xff = unlimited
    uint8_t     unknownB[7];
};

STRUCT_DEFINE(Trilogy_Loc)
{
    float y; //fixme: confirm this
    float x;
    float z;
};

STRUCT_DEFINE(Trilogy_PlayerProfile)
{
    uint32_t        crc;    // crc_calc32, negated
    char            name[30];
    char            surname[20];
    uint16_t        gender;
    uint16_t        race;
    uint16_t        class;
    uint32_t        level;
    uint32_t        experience;
    uint16_t        trainingPoints;
    int16_t         currentMana;
    uint8_t         face;
    uint8_t         unknownA[47];
    int16_t         currentHp;
    uint8_t         unknownB;
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
        
        uint8_t     stats[7];
    };
    uint8_t         languages[Lang_COUNT];
    uint8_t         unknownC[14];
    uint16_t        mainInventoryItemIds[30];
    uint32_t        mainInventoryInternalUnused[30];
    Trilogy_PPItem  mainInventoryItemProperties[30];
    Trilogy_Buff    buffs[EQP_TRILOGY_MAX_BUFFS];
    uint16_t        mainInventoryBaggedItemIds[80];
    uint16_t        cursorBaggedItemIds[10];
    Trilogy_PPItem  mainInventoryBaggedInternalUnused[80];
    Trilogy_PPItem  cursorBaggedInternalUnused[10];
    uint16_t        spellbook[EQP_TRILOGY_SPELLBOOK_SLOT_COUNT];
    uint16_t        memmedSpellIds[EQP_TRILOGY_MEMMED_SPELL_SLOT_COUNT];
    uint16_t        unknownD;
    float           y; //fixme: confirm this
    float           x;
    float           z;
    float           heading;
    char            zoneShortName[32];
    uint32_t        unknownE;   // zoneId?
    Coin            coins;
    Coin            coinsBank;
    Coin            coinsCursor;
    uint8_t         skills[EQP_TRILOGY_SKILLS_COUNT];
    uint8_t         unknownF[162];
    uint32_t        autoSplit;  // 0 or 1
    uint32_t        pvpEnabled;
    uint8_t         unknownG[12];
    uint32_t        isGM;
    uint8_t         unknownH[20];
    uint32_t        disciplinesReady;
    uint8_t         unknownI[20];
    uint32_t        hunger;
    uint32_t        thirst;
    uint8_t         unknownJ[24];
    char            bindZoneShortName[5][20];
    Trilogy_PPItem  bankInventoryItemProperties[8];
    Trilogy_PPItem  bankBaggedItemProperties[80];
    uint32_t        unknownK;
    Trilogy_Loc     bindLocs[5];
    uint8_t         unknownL[24];
    uint32_t        bankInventoryInternalUnused[8];
    uint8_t         unknownM[12];
    uint32_t        timeA;
    uint8_t         unknownN[20];
    uint16_t        bankInventoryItemIds[8];
    uint16_t        bankBaggedItemIds[80];
    uint16_t        deity;
    uint16_t        guildId;
    uint32_t        timeB;
    uint8_t         unknownO[6];
    uint8_t         fatiguePercent; // Percentage taken out of the yellow bar, i.e. (100 - fatiguePercent)
    uint16_t        unknownP;
    uint16_t        anon;
    uint8_t         guildRank;
    uint8_t         drunkeness;
    uint8_t         showEqLoadScreen;
    uint16_t        unknownQ;
    uint32_t        spellGemRefreshMilliseconds[EQP_TRILOGY_MEMMED_SPELL_SLOT_COUNT];
    uint32_t        unknownR; // Is the above field supposed to be 4 bytes lower?
    uint32_t        harmtouchRefreshMilliseconds;
    char            groupMemberNames[6][48];
    uint8_t         unknownS[3596];
};

#pragma pack()

#endif//EQP_STRUCTS_TRILOGY_H
