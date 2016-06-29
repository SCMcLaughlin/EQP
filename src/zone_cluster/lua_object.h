
#ifndef EQP_LUA_OBJECT_H
#define EQP_LUA_OBJECT_H

#include "define.h"
#include "lua_sys.h"
#include "eqp_alloc.h"
#include "timer.h"

STRUCT_DECLARE(ZC);
STRUCT_DECLARE(Zone);

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

void    zc_lua_init(ZC* zc);
void    zc_lua_clear(lua_State* L);

void    zc_lua_create_object(ZC* zc, Zone* zone, LuaObject* lobj, const char* funcName);
void    zc_lua_create_zone(ZC* zc, Zone* zone);
#define zc_lua_create_client(zc, zone, cli) zc_lua_create_object((zc), (zone), (LuaObject*)(cli), "createClient")
void    zc_lua_destroy_object(ZC* zc, LuaObject* lobj);

#define zc_lua_event(zc, zone, obj, evName) zc_lua_event_basic((zc), (zone), (LuaObject*)(obj), (evName))
void    zc_lua_event_basic(ZC* zc, Zone* zone, LuaObject* lobj, const char* eventName);

/* LuaJIT API */
EQP_API int         zc_lua_object_get_index(LuaObject* lobj);
EQP_API LuaTimer*   zc_lua_timer_create(ZC* zc, uint32_t periodMilliseconds, int luaCallback, int timerIndex, int start);
EQP_API void        zc_lua_timer_destroy(LuaTimer* timer);
EQP_API Timer*      zc_lua_timer_get_timer(LuaTimer* timer);
EQP_API int         zc_lua_timer_get_callback_index(LuaTimer* timer);
EQP_API void        zc_lua_timer_set_callback_index(LuaTimer* timer, int index);

#endif//EQP_LUA_OBJECT_H
