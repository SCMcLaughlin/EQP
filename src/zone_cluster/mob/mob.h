
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
    uint8_t     uprightState;
    uint8_t     lightLevel;
    uint8_t     texture;
    uint8_t     helmTexture;
    uint16_t    deity;
    
    float       x;
    float       y;
    float       z;
    float       heading;
    int16_t     headingRaw;
    
    int64_t     currentHp;
    int64_t     maxHp;
    int64_t     currentMana;
    int64_t     maxMana;
    int64_t     currentEndurance;
    int64_t     maxEndurance;
    
    Stats       currentStats;
    Stats       baseStats;
    
    float       currentWalkingSpeed;
    float       currentRunningSpeed;
    float       baseWalkingSpeed;
    float       baseRunningSpeed;
    
    float       currentSize;
    float       baseSize;
    
    uint16_t    bodyType;
    uint16_t    invisibilityLevel;
    uint16_t    seeInvisLevel;
    uint32_t    invisVsBodyTypeBitfields;
    int         ownerEntityId;
    
    uint32_t    primaryModelId;
    uint32_t    secondaryModelId;
    uint8_t     materials[7];
    uint32_t    tints[7];
    
    String*     name;
    String*     clientFriendlyName;
    
    Zone*       zone;
    ZC*         zoneCluster;
};

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
    MobType_Pet,
    MobType_NpcCorpse,
    MobType_ClientCorpse,
};

void    mob_init_client(R(Mob*) mob, R(ZC*) zc, R(Zone*) zone, R(Server_ClientZoning*) zoning);
void    mob_deinit(R(Mob*) mob);

#define mob_set_entity_id(mob, id) ((mob)->entityId = (id))
#define mob_set_zone_index(mob, index) ((mob)->zoneMobIndex = (index))
#define mob_zone_index(mob) ((mob)->zoneMobIndex)

#define mob_owner_entity_id(mob) ((mob)->ownerEntityId)

#define mob_is_npc(mob) (mob_get_type((mob)) == MobType_Npc)
#define mob_is_client(mob) (mob_get_type((mob)) == MobType_Client)
#define mob_is_pet(mob) (mob_get_type((mob)) == MobType_Pet)

#define             mob_name(mob) ((mob)->name)
EQP_API const char* mob_name_cstr(R(Mob*) mob);
EQP_API const char* mob_client_friendly_name_cstr(R(Mob*) mob);
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
EQP_API int8_t      mob_hp_ratio(R(Mob*) mob);
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
EQP_API float       mob_current_walking_speed(R(Mob*) mob);
EQP_API float       mob_base_walking_speed(R(Mob*) mob);
EQP_API float       mob_current_running_speed(R(Mob*) mob);
EQP_API float       mob_base_running_speed(R(Mob*) mob);
EQP_API float       mob_current_size(R(Mob*) mob);
EQP_API float       mob_base_size(R(Mob*) mob);
EQP_API uint16_t    mob_body_type(R(Mob*) mob);
EQP_API int         mob_is_invisible(R(Mob*) mob);
EQP_API int         mob_is_invisible_vs_undead(R(Mob*) mob);
EQP_API int         mob_is_invisible_vs_animals(R(Mob*) mob);
EQP_API int         mob_is_invisible_to_mob(R(Mob*) self, R(Mob*) target);
EQP_API uint8_t     mob_upright_state(R(Mob*) mob);
EQP_API uint8_t     mob_light_level(R(Mob*) mob);
EQP_API uint8_t     mob_texture(R(Mob*) mob);
EQP_API uint8_t     mob_helm_texture(R(Mob*) mob);
#define             mob_zone(mob) ((mob)->zone)
#define             mob_zone_cluster(mob) ((mob)->zoneCluster)

#define mob_get_tint(mob, index) ((mob)->tints[(index)])
#define mob_get_material(mob, index) ((mob)->materials[(index)])
#define mob_primary_model_id(mob) ((mob)->primaryModelId)
#define mob_secondary_model_id(mob) ((mob)->secondaryModelId)

#endif//EQP_MOB_H
