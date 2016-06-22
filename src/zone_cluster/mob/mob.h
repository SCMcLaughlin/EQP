
#ifndef EQP_MOB_H
#define EQP_MOB_H

#include "define.h"
#include "lua_object.h"

STRUCT_DECLARE(Zone);

STRUCT_DEFINE(Mob)
{
    // LuaObject must be the first member of this struct
    LuaObject   luaObj;
    int         mobType;
    int         entityId;
    int         zoneMobIndex;
    
    Zone*       zone;
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

#endif//EQP_MOB_H
