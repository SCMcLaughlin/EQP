
#ifndef EQP_MOB_H
#define EQP_MOB_H

#include "define.h"
#include "lua_object.h"
#include "eqp_string.h"
#include "server_structs.h"
#include "npc_prototype.h"

STRUCT_DECLARE(ZC);
STRUCT_DECLARE(Zone);

STRUCT_DEFINE(Stats)
{
    int64_t maxHp;
    int64_t maxMana;
    int64_t maxEndurance;
    int     AC;
    int     STR;
    int     STA;
    int     DEX;
    int     AGI;
    int     INT;
    int     WIS;
    int     CHA;
    int     svMagic;
    int     svFire;
    int     svCold;
    int     svPoison;
    int     svDisease;
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
    int64_t     currentMana;
    int64_t     currentEndurance;
    
    Stats       currentStats;   // Base + items + buffs
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

void    mob_init_npc(Mob* mob, ZC* zc, Zone* zone, NpcPrototype* proto, float x, float y, float z, float heading);
void    mob_init_client(Mob* mob, ZC* zc, Zone* zone, Server_ClientZoning* zoning);
void    mob_deinit(Mob* mob);

void    mob_set_position(Mob* mob, float x, float y, float z);

#define mob_set_entity_id(mob, id) ((mob)->entityId = (id))
#define mob_set_zone_index(mob, index) ((mob)->zoneMobIndex = (index))
#define mob_zone_index(mob) ((mob)->zoneMobIndex)

#define mob_owner_entity_id(mob) ((mob)->ownerEntityId)

#define mob_is_npc(mob) (mob_get_type((mob)) == MobType_Npc)
#define mob_is_client(mob) (mob_get_type((mob)) == MobType_Client)
#define mob_is_pet(mob) (mob_get_type((mob)) == MobType_Pet)

#define             mob_name(mob) ((mob)->name)
EQP_API const char* mob_name_cstr(Mob* mob);
EQP_API const char* mob_client_friendly_name_cstr(Mob* mob);
EQP_API int         mob_entity_id(Mob* mob);
EQP_API int         mob_get_type(Mob* mob);
EQP_API uint8_t     mob_level(Mob* mob);
EQP_API uint8_t     mob_class(Mob* mob);
EQP_API uint16_t    mob_base_race(Mob* mob);
EQP_API uint16_t    mob_current_race(Mob* mob);
EQP_API uint8_t     mob_base_gender(Mob* mob);
EQP_API uint8_t     mob_current_gender(Mob* mob);
EQP_API uint8_t     mob_face(Mob* mob);
EQP_API uint16_t    mob_deity(Mob* mob);
EQP_API float       mob_x(Mob* mob);
EQP_API float       mob_y(Mob* mob);
EQP_API float       mob_z(Mob* mob);
EQP_API float       mob_heading(Mob* mob);
EQP_API int8_t      mob_hp_ratio(Mob* mob);
EQP_API int64_t     mob_current_hp(Mob* mob);
EQP_API int64_t     mob_max_hp(Mob* mob);
EQP_API int64_t     mob_current_mana(Mob* mob);
EQP_API int64_t     mob_max_mana(Mob* mob);
EQP_API int64_t     mob_current_endurance(Mob* mob);
EQP_API int64_t     mob_max_endurance(Mob* mob);
EQP_API int         mob_cur_str(Mob* mob);
EQP_API int         mob_base_str(Mob* mob);
EQP_API void        mob_set_base_str(Mob* mob, int value);
EQP_API int         mob_cur_sta(Mob* mob);
EQP_API int         mob_base_sta(Mob* mob);
EQP_API void        mob_set_base_sta(Mob* mob, int value);
EQP_API int         mob_cur_dex(Mob* mob);
EQP_API int         mob_base_dex(Mob* mob);
EQP_API void        mob_set_base_dex(Mob* mob, int value);
EQP_API int         mob_cur_agi(Mob* mob);
EQP_API int         mob_base_agi(Mob* mob);
EQP_API void        mob_set_base_agi(Mob* mob, int value);
EQP_API int         mob_cur_int(Mob* mob);
EQP_API int         mob_base_int(Mob* mob);
EQP_API void        mob_set_base_int(Mob* mob, int value);
EQP_API int         mob_cur_wis(Mob* mob);
EQP_API int         mob_base_wis(Mob* mob);
EQP_API void        mob_set_base_wis(Mob* mob, int value);
EQP_API int         mob_cur_cha(Mob* mob);
EQP_API int         mob_base_cha(Mob* mob);
EQP_API void        mob_set_base_cha(Mob* mob, int value);
EQP_API float       mob_current_walking_speed(Mob* mob);
EQP_API float       mob_base_walking_speed(Mob* mob);
EQP_API void        mob_set_base_walking_speed(Mob* mob, float value);
EQP_API float       mob_current_running_speed(Mob* mob);
EQP_API float       mob_base_running_speed(Mob* mob);
EQP_API void        mob_set_base_running_speed(Mob* mob, float value);
EQP_API float       mob_current_size(Mob* mob);
EQP_API float       mob_base_size(Mob* mob);
EQP_API uint16_t    mob_body_type(Mob* mob);
EQP_API int         mob_is_invisible(Mob* mob);
EQP_API int         mob_is_invisible_vs_undead(Mob* mob);
EQP_API int         mob_is_invisible_vs_animals(Mob* mob);
EQP_API int         mob_is_invisible_to_mob(Mob* self, Mob* target);
EQP_API uint8_t     mob_upright_state(Mob* mob);
EQP_API uint8_t     mob_light_level(Mob* mob);
EQP_API uint8_t     mob_texture(Mob* mob);
EQP_API uint8_t     mob_helm_texture(Mob* mob);
#define             mob_zone(mob) ((mob)->zone)
#define             mob_zone_cluster(mob) ((mob)->zoneCluster)

#define mob_get_tint(mob, index) ((mob)->tints[(index)])
#define mob_get_material(mob, index) ((mob)->materials[(index)])
#define mob_primary_model_id(mob) ((mob)->primaryModelId)
#define mob_secondary_model_id(mob) ((mob)->secondaryModelId)

#endif//EQP_MOB_H
