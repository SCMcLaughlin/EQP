
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

ENUM_DEFINE(GenderId)
{
    Gender_Male,
    Gender_Female,
    Gender_Neuter,
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
    BodyType_None           = 0,
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

ENUM_DEFINE(ChatColorId)
{
    ChatColor_Default       = 0,
    ChatColor_DarkRed       = 1,
    ChatColor_DarkGreen     = 2,
    ChatColor_DarkYellow    = 3,
    ChatColor_DarkBlue      = 4,
    ChatColor_Purple        = 5,
    ChatColor_LightGrey     = 6,
    ChatColor_Grey          = 7,
    ChatColor_LightGreen    = 8,
    ChatColor_LightBlue     = 9,
    ChatColor_White         = 10,
    ChatColor_Grey2         = 11,
    ChatColor_Grey3         = 12,
    ChatColor_Red           = 13,
    ChatColor_Green         = 14,
    ChatColor_Yellow        = 15,
    ChatColor_Blue          = 16,
    ChatColor_Blue2         = 18,
};

ENUM_DEFINE(ChatChannelId)
{
    ChatChannel_Say                 = 256,
    ChatChannel_Tell                = 257,
    ChatChannel_Group               = 258,
    ChatChannel_Guild               = 259,
    ChatChannel_OOC                 = 260,
    ChatChannel_Auction             = 261,
    ChatChannel_Shout               = 262,
    ChatChannel_Emote               = 263,
    ChatChannel_CastSpell           = 264,
    ChatChannel_YouHitOther         = 265,
    ChatChannel_OtherHitsYou        = 266,
    ChatChannel_YouMissOther        = 267,
    ChatChannel_OtherMissesYou      = 268,
    ChatChannel_Broadcast           = 269,
    ChatChannel_Skill               = 270,
    ChatChannel_Disciplines         = 271,
    ChatChannel_Default             = 273,
    ChatChannel_MerchantOffer       = 275,
    ChatChannel_MerchantExchange    = 276,
    ChatChannel_YourDeath           = 277,
    ChatChannel_OtherDeath          = 278,
    ChatChannel_OtherHitOther       = 279,
    ChatChannel_OtherMissOther      = 280,
    ChatChannel_Who                 = 281,
    ChatChannel_Yell                = 282,
    ChatChannel_NonMelee            = 283,
    ChatChannel_SpellWornOff        = 284,
    ChatChannel_MoneySplit          = 285,
    ChatChannel_Loot                = 286,
    ChatChannel_Random              = 287,
    ChatChannel_OtherSpells         = 288,
    ChatChannel_SpellFailure        = 289,
    ChatChannel_ChatChannel         = 290,
    ChatChannel_Chat1               = 291,
    ChatChannel_Chat2               = 292,
    ChatChannel_Chat3               = 293,
    ChatChannel_Chat4               = 294,
    ChatChannel_Chat5               = 295,
    ChatChannel_Chat6               = 296,
    ChatChannel_Chat7               = 297,
    ChatChannel_Chat8               = 298,
    ChatChannel_Chat9               = 299,
    ChatChannel_Chat10              = 300,
    ChatChannel_MeleeCrit           = 301,
    ChatChannel_SpellCrit           = 302,
    ChatChannel_TooFarAway          = 303,
    ChatChannel_NPCRampage          = 304,
    ChatChannel_NPCFurry            = 305,
    ChatChannel_NPCEnrage           = 306,
    ChatChannel_EchoSay             = 307,
    ChatChannel_EchoTell            = 308,
    ChatChannel_EchoGroup           = 309,
    ChatChannel_EchoGuild           = 310,
    ChatChannel_EchoOOC             = 311,
    ChatChannel_EchoAuction         = 312,
    ChatChannel_EchoShout           = 313,
    ChatChannel_EchoEmote           = 314,
    ChatChannel_EchoChat1           = 315,
    ChatChannel_EchoChat2           = 316,
    ChatChannel_EchoChat3           = 317,
    ChatChannel_EchoChat4           = 318,
    ChatChannel_EchoChat5           = 319,
    ChatChannel_EchoChat6           = 320,
    ChatChannel_EchoChat7           = 321,
    ChatChannel_EchoChat8           = 322,
    ChatChannel_EchoChat9           = 323,
    ChatChannel_EchoChat10          = 324,
    ChatChannel_UnusedAtThisTime    = 325,
    ChatChannel_ItemTags            = 326,
    ChatChannel_RaidSay             = 327,
    ChatChannel_MyPet               = 328,
    ChatChannel_DamageShield        = 329,
};

#endif//EQP_ENUMS_H
