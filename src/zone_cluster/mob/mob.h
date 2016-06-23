
#ifndef EQP_MOB_H
#define EQP_MOB_H

#include "define.h"
#include "lua_object.h"
#include "eqp_string.h"

STRUCT_DECLARE(ZC);
STRUCT_DECLARE(Zone);

STRUCT_DEFINE(Mob)
{
    // LuaObject must be the first member of this struct
    LuaObject   luaObj;
    int         mobType;
    int         entityId;
    int         zoneMobIndex;
    
    String*     name;
    
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

void    mob_init_client(R(Mob*) mob, R(ZC*) zc, R(Zone*) zone);

#define mob_zone_cluster(mob) ((mob)->zoneCluster)

#endif//EQP_MOB_H
