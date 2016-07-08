
#include "lua_object.h"
#include "zone_cluster.h"
#include "zone.h"

#define SYS_INDEX       2
#define SYS_SCRIPT      "scripts/zone_cluster/sys/sys.lua"
#define ZC_INIT_SCRIPT  "scripts/zone_cluster/global/zone_cluster.lua"

static void zc_lua_push_sys_func(ZC* zc, lua_State* L, const char* funcName)
{
    lua_getfield(L, SYS_INDEX, funcName);
    
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 1);
        exception_throw_format(B(zc), ErrorLua, "sys.%s() does not exist; expected it to be defined in " SYS_SCRIPT, funcName);
    }
}

void zc_lua_init(ZC* zc)
{
    lua_State* L = lua_sys_open(B(zc));
    
    zc->L = L;
    
    // Load sys.lua and keep its return value at SYS_INDEX on the stack
    lua_sys_run_file(B(zc), L, SYS_SCRIPT, 1);
    
    // Create the Lua-side ZC object
    zc_lua_push_sys_func(zc, L, "createZoneCluster");
    lua_pushlightuserdata(L, zc);
    lua_sys_call(B(zc), L, 1, 0);
    
    // Run the zone cluster init script
    lua_sys_run_file_no_throw(B(zc), L, ZC_INIT_SCRIPT, 0);
}

void zc_lua_clear(lua_State* L)
{
    lua_settop(L, SYS_INDEX);
}

static void zc_lua_push(ZC* zc, lua_State* L, int index, const char* getName)
{
    zc_lua_push_sys_func(zc, L, getName);
    lua_pushinteger(L, index);
    if (!lua_sys_call_no_throw(B(zc), L, 1, 1))
        lua_pushnil(L);
}

static void zc_lua_push_object(ZC* zc, lua_State* L, int index)
{
    zc_lua_push(zc, L, index, "getObject");
}

void zc_lua_push_lua_object(ZC* zc, lua_State* L, LuaObject* lobj)
{
    zc_lua_push_object(zc, L, lobj->index);
}

static void zc_lua_push_callback(ZC* zc, lua_State* L, int index)
{
    zc_lua_push(zc, L, index, "getCallback");
}

static void zc_lua_push_timer(ZC* zc, lua_State* L, int index)
{
    zc_lua_push(zc, L, index, "getTimer");
}

void zc_lua_create_zone(ZC* zc, Zone* zone)
{
    lua_State* L    = zc->L;
    LuaObject* lobj = (LuaObject*)zone;
    
    zc_lua_push_sys_func(zc, L, "createZone");
    
    lua_pushlightuserdata(L, lobj);
    
    if (lua_sys_call_no_throw(B(zc), L, 1, 1))
    {
        lobj->index = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
}

void zc_lua_create_object(ZC* zc, Zone* zone, LuaObject* lobj, const char* funcName)
{
    lua_State* L = zc->L;
    
    zc_lua_push_sys_func(zc, L, funcName);
    
    zc_lua_push_object(zc, L, zone->luaObj.index);
    lua_pushlightuserdata(L, lobj);
    
    if (lua_sys_call_no_throw(B(zc), L, 2, 1))
    {
        lobj->index = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
}

void zc_lua_destroy_object(ZC* zc, LuaObject* lobj)
{
    lua_State* L = zc->L;
    
    zc_lua_push_sys_func(zc, L, "objectGC");
    lua_pushinteger(L, lobj->index);
    lua_sys_call_no_throw(B(zc), L, 1, 0);
}

void zc_lua_event_basic(ZC* zc, Zone* zone, LuaObject* lobj, const char* eventName)
{
    lua_State* L = zc->L;
    
    zc_lua_push_sys_func(zc, L, "eventCall");
    
    lua_pushstring(L, eventName);
    zc_lua_push_object(zc, L, zone->luaObj.index);
    zc_lua_push_object(zc, L, lobj->index);
    
    lua_sys_call_no_throw(B(zc), L, 3, 0);
}

void zc_lua_event_basic_with_other(ZC* zc, Zone* zone, LuaObject* lobj, LuaObject* lobjOther, const char* eventName)
{
    lua_State* L = zc->L;
    
    zc_lua_push_sys_func(zc, L, "eventCall");
    
    lua_pushstring(L, eventName);
    zc_lua_push_object(zc, L, zone->luaObj.index);
    zc_lua_push_object(zc, L, lobj->index);
    zc_lua_push_object(zc, L, lobjOther->index);
    
    lua_sys_call_no_throw(B(zc), L, 4, 0);
}

void zc_lua_event_prolog(ZC* zc, lua_State* L, Zone* zone, LuaObject* lobj, const char* eventName)
{
    zc_lua_push_sys_func(zc, L, "eventCall");
    
    lua_pushstring(L, eventName);
    zc_lua_push_object(zc, L, zone->luaObj.index);
    zc_lua_push_object(zc, L, lobj->index);
}

void zc_lua_event_epilog(ZC* zc, lua_State* L, int numAdditionalArgs)
{
    // Args are eventName, zone, object, plus how much was added
    lua_sys_call_no_throw(B(zc), L, 3 + numAdditionalArgs, 0);
}

void zc_lua_object_set_index(LuaObject* lobj, int index)
{
    lobj->index = index;
}

int zc_lua_object_get_index(LuaObject* lobj)
{
    return lobj->index;
}

static void zc_lua_timer_callback(Timer* timer)
{
    LuaTimer* ltimer    = timer_userdata_type(timer, LuaTimer);
    ZC* zc              = ltimer->zc;
    lua_State* L        = zc->L;
    
    zc_lua_push_callback(zc, L, ltimer->luaCallback);
    zc_lua_push_timer(zc, L, ltimer->luaObj.index);
    lua_sys_call_no_throw(B(zc), L, 1, 0);
}

LuaTimer* zc_lua_timer_create(ZC* zc, uint32_t periodMilliseconds, int luaCallback, int timerIndex, int start)
{
    LuaTimer* timer = eqp_alloc_type(B(zc), LuaTimer);
    
    timer->luaObj.index = timerIndex;
    timer->luaCallback  = luaCallback;
    timer_init(&timer->timer, &zc->timerPool, periodMilliseconds, zc_lua_timer_callback, timer, start);
    timer->zc           = zc;
    
    return timer;
}

void zc_lua_timer_destroy(LuaTimer* timer)
{
    timer_stop(&timer->timer);
    free(timer);
}

Timer* zc_lua_timer_get_timer(LuaTimer* timer)
{
    return &timer->timer;
}

int zc_lua_timer_get_callback_index(LuaTimer* timer)
{
    return timer->luaCallback;
}

void zc_lua_timer_set_callback_index(LuaTimer* timer, int index)
{
    timer->luaCallback = index;
}

#undef SYS_INDEX
#undef SYS_SCRIPT
#undef ZC_INIT_SCRIPT
