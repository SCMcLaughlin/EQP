
#include "mob.h"
#include "zone.h"
#include "zone_cluster.h"

void mob_init_client(R(Mob*) mob, R(ZC*) zc, R(Zone*) zone, R(Server_ClientZoning*) zoning)
{
    mob->mobType        = MobType_Client;
    mob->entityId       = 0;
    mob->zoneMobIndex   = -1;
    
    mob->uprightState   = Trilogy_UprightState_Standing; //fixme: make an expansion-agnostic enum for this?
    mob->texture        = 0xff;
    mob->helmTexture    = 0xff;
    
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

void mob_deinit(R(Mob*) mob)
{
    if (mob->name)
    {
        string_destroy(mob->name);
        mob->name = NULL;
    }
    
    if (mob->zoneCluster)
    {
        R(ZC*) zc = mob->zoneCluster;
        
        zc_lua_destroy_object(zc, &mob->luaObj);
    }
}

const char* mob_name_cstr(R(Mob*) mob)
{
    return string_data(mob->name);
}

const char* mob_client_friendly_name_cstr(R(Mob*) mob)
{
    return (mob->clientFriendlyName) ? string_data(mob->clientFriendlyName) : string_data(mob->name);
}

int mob_entity_id(R(Mob*) mob)
{
    return mob->entityId;
}

int mob_get_type(R(Mob*) mob)
{
    return mob->mobType;
}

uint8_t mob_level(R(Mob*) mob)
{
    return mob->level;
}

uint8_t mob_class(R(Mob*) mob)
{
    return mob->class;
}

uint16_t mob_base_race(R(Mob*) mob)
{
    return mob->baseRace;
}

uint16_t mob_current_race(R(Mob*) mob)
{
    return mob->currentRace;
}

uint8_t mob_base_gender(R(Mob*) mob)
{
    return mob->baseGender;
}

uint8_t mob_current_gender(R(Mob*) mob)
{
    return mob->currentGender;
}

uint8_t mob_face(R(Mob*) mob)
{
    return mob->face;
}

uint16_t mob_deity(R(Mob*) mob)
{
    return mob->deity;
}

float mob_x(R(Mob*) mob)
{
    return mob->x;
}

float mob_y(R(Mob*) mob)
{
    return mob->y;
}

float mob_z(R(Mob*) mob)
{
    return mob->z;
}

float mob_heading(R(Mob*) mob)
{
    return mob->heading;
}

int8_t mob_hp_ratio(R(Mob*) mob)
{
    return (mob->currentHp * 100) / mob->maxHp;
}

int64_t mob_current_hp(R(Mob*) mob)
{
    return mob->currentHp;
}

int64_t mob_max_hp(R(Mob*) mob)
{
    return mob->maxHp;
}

int64_t mob_current_mana(R(Mob*) mob)
{
    return mob->currentMana;
}

int64_t mob_max_mana(R(Mob*) mob)
{
    return mob->maxMana;
}

int64_t mob_current_endurance(R(Mob*) mob)
{
    return mob->currentEndurance;
}

int64_t mob_max_endurance(R(Mob*) mob)
{
    return mob->maxEndurance;
}

int mob_cur_str(R(Mob*) mob)
{
    return mob->currentStats.STR;
}

int mob_base_str(R(Mob*) mob)
{
    return mob->baseStats.STR;
}

void mob_set_base_str(R(Mob*) mob, int value)
{
    mob->baseStats.STR = value;
}

int mob_cur_sta(R(Mob*) mob)
{
    return mob->currentStats.STA;
}

int mob_base_sta(R(Mob*) mob)
{
    return mob->baseStats.STA;
}

void mob_set_base_sta(R(Mob*) mob, int value)
{
    mob->baseStats.STA = value;
}

int mob_cur_dex(R(Mob*) mob)
{
    return mob->currentStats.DEX;
}

int mob_base_dex(R(Mob*) mob)
{
    return mob->baseStats.DEX;
}

void mob_set_base_dex(R(Mob*) mob, int value)
{
    mob->baseStats.DEX = value;
}

int mob_cur_agi(R(Mob*) mob)
{
    return mob->currentStats.AGI;
}

int mob_base_agi(R(Mob*) mob)
{
    return mob->baseStats.AGI;
}

void mob_set_base_agi(R(Mob*) mob, int value)
{
    mob->baseStats.AGI = value;
}

int mob_cur_int(R(Mob*) mob)
{
    return mob->currentStats.INT;
}

int mob_base_int(R(Mob*) mob)
{
    return mob->baseStats.INT;
}

void mob_set_base_int(R(Mob*) mob, int value)
{
    mob->baseStats.INT = value;
}

int mob_cur_wis(R(Mob*) mob)
{
    return mob->currentStats.WIS;
}

int mob_base_wis(R(Mob*) mob)
{
    return mob->baseStats.WIS;
}

void mob_set_base_wis(R(Mob*) mob, int value)
{
    mob->baseStats.WIS = value;
}

int mob_cur_cha(R(Mob*) mob)
{
    return mob->currentStats.CHA;
}

int mob_base_cha(R(Mob*) mob)
{
    return mob->baseStats.CHA;
}

void mob_set_base_cha(R(Mob*) mob, int value)
{
    mob->baseStats.CHA = value;
}

float mob_current_walking_speed(R(Mob*) mob)
{
    return mob->currentWalkingSpeed;
}

float mob_base_walking_speed(R(Mob*) mob)
{
    return mob->baseWalkingSpeed;
}

float mob_current_running_speed(R(Mob*) mob)
{
    return mob->currentRunningSpeed;
}

float mob_base_running_speed(R(Mob*) mob)
{
    return mob->baseRunningSpeed;
}

float mob_current_size(R(Mob*) mob)
{
    return mob->currentSize;
}

float mob_base_size(R(Mob*) mob)
{
    return mob->baseSize;
}

uint16_t mob_body_type(R(Mob*) mob)
{
    return mob->bodyType;
}

int mob_is_invisible(R(Mob*) mob)
{
    return (mob->invisVsBodyTypeBitfields & (1 << (BodyType_Humanoid - 1)));
}

int mob_is_invisible_vs_undead(R(Mob*) mob)
{
    return (mob->invisVsBodyTypeBitfields & (1 << (BodyType_Undead - 1)));
}

int mob_is_invisible_vs_animals(R(Mob*) mob)
{
    return (mob->invisVsBodyTypeBitfields & (1 << (BodyType_Animal - 1)));
}

int mob_is_invisible_to_mob(R(Mob*) self, R(Mob*) target)
{
    uint16_t bodyType   = mob_body_type(target);
    int isInvis         = false;
    
    if (bodyType == 0 || bodyType >= 32)
        isInvis = mob_is_invisible(self);
    else
        isInvis = (self->invisVsBodyTypeBitfields & (1 << (bodyType - 1)));
    
    return (isInvis && self->invisibilityLevel > target->seeInvisLevel);
}

uint8_t mob_upright_state(R(Mob*) mob)
{
    return mob->uprightState;
}

uint8_t mob_light_level(R(Mob*) mob)
{
    return mob->lightLevel;
}

uint8_t mob_texture(R(Mob*) mob)
{
    return mob->texture;
}

uint8_t mob_helm_texture(R(Mob*) mob)
{
    return mob->helmTexture;
}
