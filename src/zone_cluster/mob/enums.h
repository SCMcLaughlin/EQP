
#ifndef EQP_ENUMS_H
#define EQP_ENUMS_H

#include "define.h"

ENUM_DEFINE(ClassId)
{
    Class_Warrior = 1,
    Class_Cleric,
    Class_Paladin,
    Class_Ranger,
    Class_ShadowKnight,
    Class_Druid,
    Class_Monk,
    Class_Bard,
    Class_Rogue,
    Class_Shaman,
    Class_Necromancer,
    Class_Wizard,
    Class_Magician,
    Class_Enchanter,
    Class_Beastlord,
    Class_Berserker
};

ENUM_DEFINE(RaceId)
{
    Race_Human = 1,
    Race_Barbarian,
    Race_Erudite,
    Race_WoodElf,
    Race_HighElf,
    Race_DarkElf,
    Race_HalfElf,
    Race_Dwarf,
    Race_Troll,
    Race_Ogre,
    Race_Halfling,
    Race_Gnome,
    Race_Iksar = 124
};

ENUM_DEFINE(DeityId)
{
    Deity_Bertoxxulous  = 201,
    Deity_BrellSerilis,
    Deity_CazicThule,
    Deity_ErollisiMarr,
    Deity_Bristlebane,
    Deity_Innoruuk,
    Deity_Karana,
    Deity_MithanielMarr,
    Deity_Prexus,
    Deity_Quellious,
    Deity_RallosZek,
    Deity_RodcetNife,
    Deity_SolusekRo,
    Deity_TheTribunal,
    Deity_Tunare,
    Deity_Veeshan,
    Deity_Agnostic      = 396
};

ENUM_DEFINE(BodyTypeId)
{
    BodyType_Humanoid       = 1,
    BodyType_Lycanthrope    = 2,
    BodyType_Undead         = 3,
    BodyType_Giant          = 4,
    BodyType_Construct      = 5,
    BodyType_ExtraPlanar    = 6,
    BodyType_Magical        = 7,
    BodyType_SummonedUndead = 8,
    BodyType_Untargetable   = 11,
    BodyType_Vampire        = 12,
    BodyType_Animal         = 21,
    BodyType_Insect         = 22,
    BodyType_Monster        = 23,
    BodyType_Summoned       = 24,
    BodyType_Plant          = 25,
    BodyType_Dragon         = 26,
    BodyType_Hidden         = 65,
    BodyType_InvisibleMan   = 66,
};

#endif//EQP_ENUMS_H
