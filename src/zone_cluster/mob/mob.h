
#ifndef EQP_MOB_H
#define EQP_MOB_H

#include "define.h"
#include "lua_object.h"
#include "eqp_string.h"
#include "server_structs.h"

STRUCT_DECLARE(ZC);
STRUCT_DECLARE(Zone);

STRUCT_DEFINE(Stats)
{
    int STR;
    int STA;
    int DEX;
    int AGI;
    int INT;
    int WIS;
    int CHA;
};

STRUCT_DEFINE(Mob)
{
    // LuaObject must be the first member of this struct
    LuaObject   luaObj;
    int         mobType;
    int         entityId;
    int         zoneMobIndex;
    
    uint8_t     level;
    uint8_t     class;
    uint16_t    baseRace;
    uint16_t    currentRace;
    uint8_t     baseGender;
    uint8_t     currentGender;
    uint8_t     face;
    uint16_t    deity;
    
    float       x;
    float       y;
    float       z;
    float       heading;
    
    int64_t     currentHp;
    int64_t     maxHp;
    int64_t     currentMana;
    int64_t     maxMana;
    int64_t     currentEndurance;
    int64_t     maxEndurance;
    
    Stats       currentStats;
    Stats       baseStats;
    
    String*     name;
    String*     clientFriendlyName;
    
    Zone*       zone;
    ZC*         zoneCluster;
};

/*
STRUCT_DEFINE(MobByEntityId)
{
    int     entityId;
    Mob*    mob;
};
*/

STRUCT_DEFINE(MobByPosition)
{
    float   x;
    float   y;
    float   z;
    float   aggroRadiusSquared; // -1.0f = Client, -2.0f = Pet
    Mob*    mob;
};

ENUM_DEFINE(MobType)
{
    MobType_Npc,
    MobType_Client,
    MobType_Pet
};

void    mob_init_client(R(Mob*) mob, R(ZC*) zc, R(Zone*) zone, R(Server_ClientZoning*) zoning);
void    mob_deinit(R(Mob*) mob);

#define mob_set_entity_id(mob, id) ((mob)->entityId = (id))
#define mob_set_zone_index(mob, index) ((mob)->zoneMobIndex = (index))
#define mob_zone_index(mob) ((mob)->zoneMobIndex)

#define mob_is_npc(mob) (mob_get_type((mob)) == MobType_Npc)
#define mob_is_client(mob) (mob_get_type((mob)) == MobType_Client)
#define mob_is_pet(mob) (mob_get_type((mob)) == MobType_Pet)

#define             mob_name(mob) ((mob)->name)
EQP_API const char* mob_name_cstr(R(Mob*) mob);
EQP_API int         mob_entity_id(R(Mob*) mob);
EQP_API int         mob_get_type(R(Mob*) mob);
EQP_API uint8_t     mob_level(R(Mob*) mob);
EQP_API uint8_t     mob_class(R(Mob*) mob);
EQP_API uint16_t    mob_base_race(R(Mob*) mob);
EQP_API uint16_t    mob_current_race(R(Mob*) mob);
EQP_API uint8_t     mob_base_gender(R(Mob*) mob);
EQP_API uint8_t     mob_current_gender(R(Mob*) mob);
EQP_API uint8_t     mob_face(R(Mob*) mob);
EQP_API uint16_t    mob_deity(R(Mob*) mob);
EQP_API float       mob_x(R(Mob*) mob);
EQP_API float       mob_y(R(Mob*) mob);
EQP_API float       mob_z(R(Mob*) mob);
EQP_API float       mob_heading(R(Mob*) mob);
EQP_API int64_t     mob_current_hp(R(Mob*) mob);
EQP_API int64_t     mob_max_hp(R(Mob*) mob);
EQP_API int64_t     mob_current_mana(R(Mob*) mob);
EQP_API int64_t     mob_max_mana(R(Mob*) mob);
EQP_API int64_t     mob_current_endurance(R(Mob*) mob);
EQP_API int64_t     mob_max_endurance(R(Mob*) mob);
EQP_API int         mob_cur_str(R(Mob*) mob);
EQP_API int         mob_base_str(R(Mob*) mob);
EQP_API int         mob_cur_sta(R(Mob*) mob);
EQP_API int         mob_base_sta(R(Mob*) mob);
EQP_API int         mob_cur_dex(R(Mob*) mob);
EQP_API int         mob_base_dex(R(Mob*) mob);
EQP_API int         mob_cur_agi(R(Mob*) mob);
EQP_API int         mob_base_agi(R(Mob*) mob);
EQP_API int         mob_cur_int(R(Mob*) mob);
EQP_API int         mob_base_int(R(Mob*) mob);
EQP_API int         mob_cur_wis(R(Mob*) mob);
EQP_API int         mob_base_wis(R(Mob*) mob);
EQP_API int         mob_cur_cha(R(Mob*) mob);
EQP_API int         mob_base_cha(R(Mob*) mob);
#define             mob_zone(mob) ((mob)->zone)
#define             mob_zone_cluster(mob) ((mob)->zoneCluster)

#endif//EQP_MOB_H
