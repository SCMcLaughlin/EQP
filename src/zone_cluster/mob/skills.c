
#include "skills.h"

void skills_init(R(Skills*) skills)
{
    // The Mob has already been zeroed when this is called, just need to set default non-zero things
    
    skills->language[Lang_CommonTongue] = 100;
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
