
#ifndef EQP_STRUCTS_TRILOGY_H
#define EQP_STRUCTS_TRILOGY_H

#include "define.h"
#include "enums.h"
#include "skills.h"
#include "defines_trilogy.h"
#include "structs.h"
#include "player_profile.h"

#pragma pack(1)

STRUCT_DEFINE(Trilogy_ZoneEntryFromClient)
{
    uint32_t    checksum;
    char        characterName[32];
};

STRUCT_DEFINE(Trilogy_ZoneEntry)
{
    uint32_t    crc;
    uint8_t     unknownA;
    char        name[30];
    char        zoneShortName[20];
    uint8_t     unknownB;
    float       y;
    float       x;
    float       z;
    float       heading;
    uint8_t     unknownC[76];
    uint16_t    guildId;
    uint8_t     unknownD[7];    // [6]: 2 makes the client insta-die on zoning in
    uint8_t     class;
    uint16_t    race;
    uint8_t     gender;
    uint8_t     level;
    uint8_t     unknownE[2];
    uint8_t     isPvP;
    uint8_t     unknownF[2];
    uint8_t     face;
    uint8_t     helmMaterial;
    uint8_t     unknownG[6];
    uint8_t     secondaryWeaponModelId;
    uint8_t     primaryWeaponModelId;
    uint8_t     unknownH[3];
    uint32_t    helmColor;
    uint8_t     unknownI[32];
    uint8_t     texture; // 0xff for standard client gear
    uint8_t     unknownJ[19];
    float       walkingSpeed;
    float       runningSpeed;
    uint8_t     unknownK[12];
    uint8_t     anon;
    uint8_t     unknownL[23];
    char        surname[20];
    uint8_t     unknownM[2];
    uint16_t    deity;
    uint8_t     unknownN[8];
};

STRUCT_DEFINE(Trilogy_Weather)
{
    int type;
    int intensity;
};

STRUCT_DEFINE(Trilogy_ZoneInfo)
{
    char        characterName[30];
    char        zoneShortName[20];
    char        zoneLongName[180];
    uint8_t     zoneType;                   // Default 0xff
    uint8_t     fogRed[4];
    uint8_t     fogGreen[4];
    uint8_t     fogBlue[4];
    uint8_t     unknownA;
    float       fogMinClippingDistance[4];
    float       fogMaxClippingDistance[4];
    float       gravity;                    // Default 0.4f
    uint8_t     unknownB[50];
    uint16_t    skyType;
    uint8_t     unknownC[8];
    float       unknownD;                   // Default 0.75
    float       safeSpotX;
    float       safeSpotY;
    float       safeSpotZ;
    float       safeSpotHeading;            // Assumed
    float       minZ;                       // If a client falls below this, they are sent to the safe spot; default -32000.0f
    float       minClippingDistance;        // Default 1000.0f
    float       maxClippingDistance;        // Default 20000.0f
    uint8_t     unknownE[32];
};

STRUCT_DEFINE(Trilogy_SpawnAppearance)
{
    uint16_t    entityId;
    uint16_t    unknownA;
    uint16_t    type;
    uint16_t    unknownB;
    uint32_t    value;
};

STRUCT_DEFINE(Trilogy_Spawn)
{
    uint32_t    unknownA;
    float       size;
    float       walkingSpeed;
    float       runningSpeed;
    uint32_t    tints[7];
    uint8_t     unknownB[9];    // Equip-related ?
    int8_t      heading;
    int8_t      headingDelta;
    int16_t     y;
    int16_t     x;
    int16_t     z;
    int         yDelta  : 10;
    int         unusedA : 1;
    int         zDelta  : 10;
    int         unusedB : 1;
    int         xDelta  : 10;
    uint8_t     unknownC;
    uint16_t    entityId;
    uint16_t    bodyType;
    uint16_t    ownerEntityId;  // 0 = not a pet
    int16_t     hpPercent;
    uint16_t    guildId;
    uint8_t     race;
    uint8_t     mobType;        // 0 = client, 1 = npc, 2 = client corpse, 3 = npc corpse, 10 = self
    uint8_t     class;
    uint8_t     gender;
    uint8_t     level;
    uint8_t     isInvisible;
    uint8_t     unknownD;
    uint8_t     isPvP;
    uint8_t     uprightState;   // Trilogy_UprightState_Standing and such
    uint8_t     light;
    uint8_t     anon;
    uint8_t     isAfk;
    uint8_t     unknownE;
    uint8_t     isLinkdead;
    uint8_t     isGM;
    uint8_t     unknownF;
    uint8_t     texture;
    uint8_t     helmTexture;
    uint8_t     unknownG;
    uint8_t     materials[9];
    char        name[30];
    char        surname[20];
    uint8_t     guildRank;
    uint8_t     unknownH;
    uint16_t    deity;  //fixme: may be uint8?
    uint8_t     unknownI[10];
};

STRUCT_DEFINE(Trilogy_CustomMessage)
{
    uint32_t    chatChannel;
    char        message[0];
};

STRUCT_DEFINE(Trilogy_PositionUpdate)
{
    uint16_t    entityId;
    int8_t      movementType;   // 0 = strafing
    int8_t      heading;
    int8_t      deltaHeading;
    int16_t     y;
    int16_t     x;
    int16_t     z;
    int         deltaY      : 10;
    int         paddingA    : 1;
    int         deltaZ      : 10;
    int         paddingB    : 1;
    int         deltaX      : 10;
};

STRUCT_DEFINE(Trilogy_PositionUpdateSet)
{
    uint32_t                count;
    Trilogy_PositionUpdate  updates[0];
};

STRUCT_DEFINE(Trilogy_ItemBasic)
{
    int8_t      STR;
    int8_t      STA;
    int8_t      CHA;
    int8_t      DEX;
    int8_t      INT;
    int8_t      AGI;
    int8_t      WIS;
    int8_t      MR;
    int8_t      FR;
    int8_t      CR;
    int8_t      DR;
    int8_t      PR;
    int8_t      hp;
    int8_t      mana;
    int8_t      AC;
    union
    {
        uint8_t     isStackable;
        uint8_t     hasUnlimitedCharges;
    };
    uint8_t     isTestItem;
    uint8_t     light;
    uint8_t     delay;
    uint8_t     damage;
    uint8_t     clickyType; // 0 = none/proc, 1 = unrestricted clicky, 2 = worn, 3 = unrestricted expendable, 4 = must-equip clicky, 5 = class-restricted clicky
    uint8_t     range;
    uint8_t     skill;
    uint8_t     isMagic;
    uint8_t     clickableLevel;
    uint8_t     material;
    uint16_t    unknownA;
    uint32_t    tint;
    uint16_t    unknownB;
    uint16_t    spellId;
    uint32_t    classesBitfield;
    union
    {
        struct
        {
            uint32_t    racesBitfield;
            uint8_t     consumableType; // 3 for clickies (and procs..?)
        };
        struct
        {
            uint8_t     type;
            uint8_t     capacity;
            uint8_t     isOpen;
            uint8_t     containableSize;
            uint8_t     weightReductionPercent;
        } bag;
    };
    union
    {
        uint8_t procLevel;
        uint8_t hastePercent;
    };
    uint8_t     charges;
    uint8_t     effectType; // 0 = none/proc, 1 = unrestricted clicky, 2 = worn, 3 = unrestricted expendable, 4 = must-equip clicky, 5 = class-restricted clicky
    uint16_t    clickySpellId;
    uint8_t     unknownC[10];
    uint32_t    castingTime;
    uint8_t     unknownD[28];
    uint8_t     recommendedLevel;
    uint8_t     unknownE[23];
    uint32_t    deityBitfield;
};

STRUCT_DEFINE(Trilogy_ItemBook)
{
    uint8_t     isBook;
    uint16_t    type;
    char        fileName[15];
    uint8_t     unknownA[102];
};

STRUCT_DEFINE(Trilogy_Item)
{
    char        name[35];
    char        lore[60];
    char        model[6];
    uint16_t    typeFlag;       // 0 = scroll, 0x3336 = basic, 0x5400 = container, 0x7669 = book
    uint8_t     unknownA[22];
    uint8_t     weight;
    uint8_t     isPermanent;    // 0 = no rent, 1/255 = permanent
    uint8_t     isDroppable;    // 0 = no drop
    uint8_t     size;
    uint8_t     itemType;       // 0 = basic, 1 = container, 2 = book/scroll
    uint16_t    itemId;
    uint16_t    icon;
    uint16_t    currentSlot;
    uint32_t    slotsBitfield;
    uint32_t    cost;
    uint8_t     unknownB[16];
    uint32_t    instanceId;
    uint8_t     isDroppableRoleplayServer;
    uint8_t     unknownC[7];
    union
    {
        Trilogy_ItemBasic   basic;
        Trilogy_ItemBook    book;
    };
};

#pragma pack()

#endif//EQP_STRUCTS_TRILOGY_H
