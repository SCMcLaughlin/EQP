
#ifndef EQP_SKILLS_H
#define EQP_SKILLS_H

#include "define.h"
#include "enums.h"

ENUM_DEFINE(SkillId)
{
    Skill_1HBlunt,
    Skill_1HSlashing,
    Skill_2HBlunt,
    Skill_2HSlashing,
    Skill_Abjuration,
    Skill_Alteration,
    Skill_ApplyPoison,
    Skill_Archery,
    Skill_Backstab,
    Skill_BindWound,
    Skill_Bash,
    Skill_Block,
    Skill_BrassInstruments,
    Skill_Channeling,
    Skill_Conjuration,
    Skill_Defense,
    Skill_Disarm,
    Skill_DisarmTraps,
    Skill_Divination,
    Skill_Dodge,
    Skill_DoubleAttack,
    Skill_DragonPunch,
    Skill_DualWield,
    Skill_EagleStrike,
    Skill_Evocation,
    Skill_FeignDeath,
    Skill_FlyingKick,
    Skill_Forage,
    Skill_HandToHand,
    Skill_Hide,
    Skill_Kick,
    Skill_Meditate,
    Skill_Mend,
    Skill_Offense,
    Skill_Parry,
    Skill_PickLock,
    Skill_1HPiercing,
    Skill_Riposte,
    Skill_RoundKick,
    Skill_SafeFall,
    Skill_SenseHeading,
    Skill_Singing,
    Skill_Sneak,
    Skill_SpecializeAbjuration,
    Skill_SpecializeAlteration,
    Skill_SpecializeConjuration,
    Skill_SpecializeDivination,
    Skill_SpecializeEvocation,
    Skill_PickPockets,
    Skill_StringedInstruments,
    Skill_Swimming,
    Skill_Throwing,
    Skill_TigerClaw,
    Skill_Tracking,
    Skill_WindInstruments,
    Skill_Fishing,
    Skill_MakePoison,
    Skill_Tinkering,
    Skill_Research,
    Skill_Alchemy,
    Skill_Baking,
    Skill_Tailoring,
    Skill_SenseTraps,
    Skill_Blacksmithing,
    Skill_Fletching,
    Skill_Brewing,
    Skill_AlcoholTolerance,
    Skill_Begging,
    Skill_JewelryMaking,
    Skill_Pottery,
    Skill_PercussionInstruments,
    Skill_Intimidation,
    Skill_Berserking,
    Skill_Taunt,
    Skill_COUNT
};

ENUM_DEFINE(LanguageId)
{
    Lang_CommonTongue,
    Lang_Barbarian,
    Lang_Erudian,
    Lang_Elvish,
    Lang_DarkElvish,
    Lang_Dwarvish,
    Lang_Troll,
    Lang_Ogre,
    Lang_Gnomish,
    Lang_Halfling,
    Lang_TheivesCant,
    Lang_OldErudian,
    Lang_ElderElvish,
    Lang_Froglok,
    Lang_Goblin,
    Lang_Gnoll,
    Lang_CombineTongue,
    Lang_ElderTierDal,
    Lang_Lizardman,
    Lang_Orcish,
    Lang_Faerie,
    Lang_Dragon,
    Lang_ElderDragon,
    Lang_DarkSpeech,
    Lang_COUNT
};

#define EQP_DB_LANGUAGE_SKILL_OFFSET (Skill_COUNT + 1)   /* Languages are stored as skills in the database, using this offset */

STRUCT_DEFINE(Skills)
{
    uint8_t skill[Skill_COUNT];
    uint8_t language[Lang_COUNT];
};

void    skills_preinit(R(Skills*) skills);
void    skills_init(R(Skills*) skills, uint32_t class, uint32_t level, uint32_t race);
void    skills_set_from_db(R(Skills*) skills, uint32_t skillId, uint32_t value);

#endif//EQP_SKILLS_H
