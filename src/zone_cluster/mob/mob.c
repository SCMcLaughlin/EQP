
#include "mob.h"
#include "zone.h"
#include "zone_cluster.h"

void mob_init_client(R(Mob*) mob, R(ZC*) zc, R(Zone*) zone, R(Server_ClientZoning*) zoning)
{
    mob->mobType        = MobType_Client;
    mob->entityId       = -1;
    mob->zoneMobIndex   = -1;
    
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

int mob_cur_sta(R(Mob*) mob)
{
    return mob->currentStats.STA;
}

int mob_base_sta(R(Mob*) mob)
{
    return mob->baseStats.STA;
}

int mob_cur_dex(R(Mob*) mob)
{
    return mob->currentStats.DEX;
}

int mob_base_dex(R(Mob*) mob)
{
    return mob->baseStats.DEX;
}

int mob_cur_agi(R(Mob*) mob)
{
    return mob->currentStats.AGI;
}

int mob_base_agi(R(Mob*) mob)
{
    return mob->baseStats.AGI;
}

int mob_cur_int(R(Mob*) mob)
{
    return mob->currentStats.INT;
}

int mob_base_int(R(Mob*) mob)
{
    return mob->baseStats.INT;
}

int mob_cur_wis(R(Mob*) mob)
{
    return mob->currentStats.WIS;
}

int mob_base_wis(R(Mob*) mob)
{
    return mob->baseStats.WIS;
}

int mob_cur_cha(R(Mob*) mob)
{
    return mob->currentStats.CHA;
}

int mob_base_cha(R(Mob*) mob)
{
    return mob->baseStats.CHA;
}
