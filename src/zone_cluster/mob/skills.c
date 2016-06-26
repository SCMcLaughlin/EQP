
#include "skills.h"

#define SKILL_CANT_LEARN            0xff
#define SKILL_HAVENT_TRAINED_YET    0xfe

void skills_preinit(R(Skills*) skills)
{
    uint32_t i;
    
    static const uint8_t skillsThatDefaultToZeroUniversally[] = {
        Skill_1HBlunt,
        Skill_BindWound,
        Skill_Defense,
        Skill_HandToHand,
        Skill_Offense,
        Skill_SenseHeading,
        Skill_Swimming, // Except for Iksars, which will be handled later...
        Skill_Throwing,
        Skill_Fishing,
        Skill_Baking,
        Skill_Tailoring,
        Skill_Blacksmithing,
        Skill_Fletching,
        Skill_Brewing,
        Skill_AlcoholTolerance,
        Skill_Begging,
        Skill_JewelryMaking,
        Skill_Pottery
    };
    
    memset(skills->skill, SKILL_HAVENT_TRAINED_YET, sizeof(skills->skill));
    
    for (i = 0; i < sizeof(skillsThatDefaultToZeroUniversally); i++)
    {
        skills->skill[skillsThatDefaultToZeroUniversally[i]] = 0;
    }
    
    skills->language[Lang_CommonTongue] = 100;
}

void skills_init(R(Skills*) skills, uint32_t class, uint32_t level, uint32_t race)
{
    (void)class;
    (void)level;
    (void)race;
    
    if (class != Class_Monk)
    {
        skills->skill[Skill_DragonPunch] = SKILL_CANT_LEARN;
        skills->skill[Skill_EagleStrike] = SKILL_CANT_LEARN;
        skills->skill[Skill_FeignDeath] = SKILL_CANT_LEARN;
        skills->skill[Skill_FlyingKick] = SKILL_CANT_LEARN;
        skills->skill[Skill_RoundKick] = SKILL_CANT_LEARN;
        skills->skill[Skill_TigerClaw] = SKILL_CANT_LEARN;
    }
    
    if (class != Class_Rogue)
    {
        skills->skill[Skill_ApplyPoison] = SKILL_CANT_LEARN;
        skills->skill[Skill_Backstab] = SKILL_CANT_LEARN;
        skills->skill[Skill_DisarmTraps] = SKILL_CANT_LEARN;
        skills->skill[Skill_PickLock] = SKILL_CANT_LEARN;
        skills->skill[Skill_PickPockets] = SKILL_CANT_LEARN;
        skills->skill[Skill_MakePoison] = SKILL_CANT_LEARN;
    }
    
    if (class != Class_Monk && class != Class_Beastlord)
        skills->skill[Skill_Block] = SKILL_CANT_LEARN;
    
    if (class < Class_Necromancer || class > Class_Enchanter)
        skills->skill[Skill_Research] = SKILL_CANT_LEARN;
    
    if (class != Class_Shaman)
        skills->skill[Skill_Alchemy] = SKILL_CANT_LEARN;
    
    if (race != Race_Gnome)
        skills->skill[Skill_Tinkering] = SKILL_CANT_LEARN;
    
    if (race == Race_Iksar)
        skills->skill[Skill_Swimming] = 100;
}

void skills_set_from_db(R(Skills*) skills, uint32_t skillId, uint32_t value)
{
    if (skillId >= EQP_DB_LANGUAGE_SKILL_OFFSET)
    {
        skillId -= EQP_DB_LANGUAGE_SKILL_OFFSET;
        
        if (skillId >= Lang_COUNT)
            return;
        
        skills->language[skillId] = value;
    }
    
    skills->skill[skillId] = value;
}

#undef SKILL_CANT_LEARN
#undef SKILL_HAVENT_TRAINED_YET
