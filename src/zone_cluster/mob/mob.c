
#include "mob.h"
#include "zone.h"
#include "zone_cluster.h"

void mob_init_client(Mob* mob, ZC* zc, Zone* zone, Server_ClientZoning* zoning)
{
    mob->mobType        = MobType_Client;
    mob->zoneMobIndex   = -1;
    
    mob->uprightState   = Trilogy_UprightState_Standing; //fixme: make an expansion-agnostic enum for this?
    mob->texture        = 0xff;
    mob->helmTexture    = 0xff;
    
    // Temporary, just to make sure we don't try to divide by zero anywhere...
    mob->maxHp          = 100;
    mob->maxMana        = 100;
    mob->maxEndurance   = 100;
    
    mob->currentWalkingSpeed    = 0.46f;
    mob->currentRunningSpeed    = 0.7f;
    mob->baseWalkingSpeed       = 0.46f;
    mob->baseRunningSpeed       = 0.7f;
    
    mob->currentSize    = 4.0f; // Correct these when our base race is set
    mob->baseSize       = 4.0f;
    
    mob->bodyType       = BodyType_Humanoid;
    
    mob->name           = string_create_from_cstr(B(zc), zoning->characterName, strlen(zoning->characterName));
    
    mob->zone           = zone;
    mob->zoneCluster    = zc;
}

void mob_deinit(Mob* mob)
{
    if (mob->name)
    {
        string_destroy(mob->name);
        mob->name = NULL;
    }
    
    if (mob->zoneCluster)
    {
        ZC* zc  = mob->zoneCluster;
        
        zc_lua_destroy_object(zc, &mob->luaObj);
    }
}

const char* mob_name_cstr(Mob* mob)
{
    return string_data(mob->name);
}

const char* mob_client_friendly_name_cstr(Mob* mob)
{
    return (mob->clientFriendlyName) ? string_data(mob->clientFriendlyName) : string_data(mob->name);
}

int mob_entity_id(Mob* mob)
{
    return mob->entityId;
}

int mob_get_type(Mob* mob)
{
    return mob->mobType;
}

uint8_t mob_level(Mob* mob)
{
    return mob->level;
}

uint8_t mob_class(Mob* mob)
{
    return mob->class;
}

uint16_t mob_base_race(Mob* mob)
{
    return mob->baseRace;
}

uint16_t mob_current_race(Mob* mob)
{
    return mob->currentRace;
}

uint8_t mob_base_gender(Mob* mob)
{
    return mob->baseGender;
}

uint8_t mob_current_gender(Mob* mob)
{
    return mob->currentGender;
}

uint8_t mob_face(Mob* mob)
{
    return mob->face;
}

uint16_t mob_deity(Mob* mob)
{
    return mob->deity;
}

float mob_x(Mob* mob)
{
    return mob->x;
}

float mob_y(Mob* mob)
{
    return mob->y;
}

float mob_z(Mob* mob)
{
    return mob->z;
}

float mob_heading(Mob* mob)
{
    return mob->heading;
}

int8_t mob_hp_ratio(Mob* mob)
{
    return (mob->currentHp * 100) / mob->maxHp;
}

int64_t mob_current_hp(Mob* mob)
{
    return mob->currentHp;
}

int64_t mob_max_hp(Mob* mob)
{
    return mob->maxHp;
}

int64_t mob_current_mana(Mob* mob)
{
    return mob->currentMana;
}

int64_t mob_max_mana(Mob* mob)
{
    return mob->maxMana;
}

int64_t mob_current_endurance(Mob* mob)
{
    return mob->currentEndurance;
}

int64_t mob_max_endurance(Mob* mob)
{
    return mob->maxEndurance;
}

int mob_cur_str(Mob* mob)
{
    return mob->currentStats.STR;
}

int mob_base_str(Mob* mob)
{
    return mob->baseStats.STR;
}

void mob_set_base_str(Mob* mob, int value)
{
    mob->baseStats.STR = value;
}

int mob_cur_sta(Mob* mob)
{
    return mob->currentStats.STA;
}

int mob_base_sta(Mob* mob)
{
    return mob->baseStats.STA;
}

void mob_set_base_sta(Mob* mob, int value)
{
    mob->baseStats.STA = value;
}

int mob_cur_dex(Mob* mob)
{
    return mob->currentStats.DEX;
}

int mob_base_dex(Mob* mob)
{
    return mob->baseStats.DEX;
}

void mob_set_base_dex(Mob* mob, int value)
{
    mob->baseStats.DEX = value;
}

int mob_cur_agi(Mob* mob)
{
    return mob->currentStats.AGI;
}

int mob_base_agi(Mob* mob)
{
    return mob->baseStats.AGI;
}

void mob_set_base_agi(Mob* mob, int value)
{
    mob->baseStats.AGI = value;
}

int mob_cur_int(Mob* mob)
{
    return mob->currentStats.INT;
}

int mob_base_int(Mob* mob)
{
    return mob->baseStats.INT;
}

void mob_set_base_int(Mob* mob, int value)
{
    mob->baseStats.INT = value;
}

int mob_cur_wis(Mob* mob)
{
    return mob->currentStats.WIS;
}

int mob_base_wis(Mob* mob)
{
    return mob->baseStats.WIS;
}

void mob_set_base_wis(Mob* mob, int value)
{
    mob->baseStats.WIS = value;
}

int mob_cur_cha(Mob* mob)
{
    return mob->currentStats.CHA;
}

int mob_base_cha(Mob* mob)
{
    return mob->baseStats.CHA;
}

void mob_set_base_cha(Mob* mob, int value)
{
    mob->baseStats.CHA = value;
}

float mob_current_walking_speed(Mob* mob)
{
    return mob->currentWalkingSpeed;
}

float mob_base_walking_speed(Mob* mob)
{
    return mob->baseWalkingSpeed;
}

float mob_current_running_speed(Mob* mob)
{
    return mob->currentRunningSpeed;
}

float mob_base_running_speed(Mob* mob)
{
    return mob->baseRunningSpeed;
}

float mob_current_size(Mob* mob)
{
    return mob->currentSize;
}

float mob_base_size(Mob* mob)
{
    return mob->baseSize;
}

uint16_t mob_body_type(Mob* mob)
{
    return mob->bodyType;
}

int mob_is_invisible(Mob* mob)
{
    return (mob->invisVsBodyTypeBitfields & (1 << (BodyType_Humanoid - 1)));
}

int mob_is_invisible_vs_undead(Mob* mob)
{
    return (mob->invisVsBodyTypeBitfields & (1 << (BodyType_Undead - 1)));
}

int mob_is_invisible_vs_animals(Mob* mob)
{
    return (mob->invisVsBodyTypeBitfields & (1 << (BodyType_Animal - 1)));
}

int mob_is_invisible_to_mob(Mob* self, Mob* target)
{
    uint16_t bodyType   = mob_body_type(target);
    int isInvis         = false;
    
    if (bodyType == 0 || bodyType >= 32)
        isInvis = mob_is_invisible(self);
    else
        isInvis = (self->invisVsBodyTypeBitfields & (1 << (bodyType - 1)));
    
    return (isInvis && self->invisibilityLevel > target->seeInvisLevel);
}

uint8_t mob_upright_state(Mob* mob)
{
    return mob->uprightState;
}

uint8_t mob_light_level(Mob* mob)
{
    return mob->lightLevel;
}

uint8_t mob_texture(Mob* mob)
{
    return mob->texture;
}

uint8_t mob_helm_texture(Mob* mob)
{
    return mob->helmTexture;
}
