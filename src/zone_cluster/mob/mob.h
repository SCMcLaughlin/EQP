
#ifndef EQP_MOB_H
#define EQP_MOB_H

#include "define.h"
#include "lua_object.h"

STRUCT_DEFINE(Mob)
{
    // LuaObject must be the first member of this struct
    LuaObject   luaObj;
};

#endif//EQP_MOB_H
