
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
    uint16_t    race;
    uint8_t     gender;
    uint8_t     face;
    uint8_t     deity;
    
    float       x;
    float       y;
    float       z;
    
    int64_t     currentHp;
    int64_t     currentMana;
    int64_t     currentEndurance;
    
    Stats       currentStats;
    Stats       baseStats;
    
    String*     name;
    String*     clientFriendlyName;
    
    Zone*       zone;
    ZC*         zoneCluster;
};

STRUCT_DEFINE(MobByEntityId)
{
    int     entityId;
    Mob*    mob;
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
    MobType_Pet
};

void    mob_init_client(R(Mob*) mob, R(ZC*) zc, R(Zone*) zone, R(Server_ClientZoning*) zoning);
void    mob_deinit(R(Mob*) mob);

#define mob_name(mob) ((mob)->name)
#define mob_zone_cluster(mob) ((mob)->zoneCluster)

#endif//EQP_MOB_H
