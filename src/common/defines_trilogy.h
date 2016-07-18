
#ifndef EQP_DEFINES_TRILOGY_H
#define EQP_DEFINES_TRILOGY_H

#include "define.h"

#define EQP_TRILOGY_MAX_BUFFS               15
#define EQP_TRILOGY_SKILLS_COUNT            74
#define EQP_TRILOGY_LANGUAGES_COUNT         24
#define EQP_TRILOGY_SPELLBOOK_SLOT_COUNT    256
#define EQP_TRILOGY_MEMMED_SPELL_SLOT_COUNT 8

ENUM_DEFINE(Trilogy_SpawnAppearanceType)
{
    Trilogy_SpawnAppearance_SendToBind      = 0,
    Trilogy_SpawnAppearance_LevelChange     = 1,
    Trilogy_SpawnAppearance_Invisible       = 3,
    Trilogy_SpawnAppearance_PvP             = 4,
    Trilogy_SpawnAppearance_Light           = 5,
    Trilogy_SpawnAppearance_UprightState    = 14,
    Trilogy_SpawnAppearance_Sneaking        = 15,
    Trilogy_SpawnAppearance_SetEntityId     = 16,
    Trilogy_SpawnAppearance_HpRegenTick     = 17,   // Sent by the client to tell us when to regen... very much ignored by us
    Trilogy_SpawnAppearance_Linkdead        = 18,
    Trilogy_SpawnAppearance_FlyMode         = 19,   // 0 = none, 1 = flymode, 2 = levitate
    Trilogy_SpawnAppearance_GMFlag          = 20,
    Trilogy_SpawnAppearance_AnonSetting     = 21,   // 0 = none, 1 = anon, 2 = roleplay, 3 = anon roleplay
    Trilogy_SpawnAppearance_GuildId         = 22,
    Trilogy_SpawnAppearance_GuildRank       = 23,
    Trilogy_SpawnAppearance_AFK             = 24,
    Trilogy_SpawnAppearance_AutoSplit       = 28,
    Trilogy_SpawnAppearance_Size            = 29,
    Trilogy_SpawnAppearance_BecomeNPC       = 30,
};

ENUM_DEFINE(Trilogy_UprightStateType) // Values for Trilogy_SpawnAppearance_UprightState
{
    Trilogy_UprightState_Standing       = 100,
    Trilogy_UprightState_LoseControl    = 102,
    Trilogy_UprightState_Looting        = 105,
    Trilogy_UprightState_Sitting        = 110,
    Trilogy_UprightState_Ducking        = 111,
    Trilogy_UprightState_Unconscious    = 115,
};

#endif//EQP_DEFINES_TRILOGY_H
