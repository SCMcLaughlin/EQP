
#ifndef EQP_LUA_OBJECT_H
#define EQP_LUA_OBJECT_H

#include "define.h"
#include "lua_sys.h"
#include "eqp_alloc.h"
#include "timer.h"

STRUCT_DECLARE(ZC);

STRUCT_DEFINE(LuaObject)
{
    int index;
};

STRUCT_DEFINE(LuaTimer)
{
    LuaObject   luaObj;
    int         luaCallback;
    Timer       timer;
    ZC*         zc;
};

void    zc_lua_init(R(ZC*) zc);
void    zc_lua_clear(R(lua_State*) L);

void    zc_lua_create_object(R(ZC*) zc, R(lua_State*) L, R(LuaObject*) lobj, R(const char*) funcName);

/* LuaJIT API */
EQP_API int         zc_lua_object_get_index(R(LuaObject*) lobj);
EQP_API LuaTimer*   zc_lua_timer_create(R(ZC*) zc, uint32_t periodMilliseconds, int luaCallback, int timerIndex, int start);
EQP_API void        zc_lua_timer_destroy(R(LuaTimer*) timer);
EQP_API Timer*      zc_lua_timer_get_timer(R(LuaTimer*) timer);
EQP_API int         zc_lua_timer_get_callback_index(R(LuaTimer*) timer);
EQP_API void        zc_lua_timer_set_callback_index(R(LuaTimer*) timer, int index);

#endif//EQP_LUA_OBJECT_H
