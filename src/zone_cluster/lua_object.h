
#ifndef EQP_LUA_OBJECT_H
#define EQP_LUA_OBJECT_H

#include "define.h"
#include "lua_sys.h"

STRUCT_DECLARE(ZC);

STRUCT_DEFINE(LuaObject)
{
    int index;
};

void    zc_lua_init(R(ZC*) zc);
void    zc_lua_clear(R(lua_State*) L);

void    zc_lua_create_object(R(ZC*) zc, R(lua_State*) L, R(LuaObject*) lobj, R(const char*) funcName);

/* LuaJIT API */
EQP_API void    zc_lua_object_update_index(R(LuaObject*) lobj, int index);

#endif//EQP_LUA_OBJECT_H
