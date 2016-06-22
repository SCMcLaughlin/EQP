
#include "lua_object.h"
#include "zone_cluster.h"

#define SYS_INDEX       2
#define SYS_SCRIPT      "scripts/zone_cluster/sys/sys.lua"
#define ZC_INIT_SCRIPT  "scripts/zone_cluster/init/zone_cluster_init.lua"

static void zc_lua_push_sys_func(R(ZC*) zc, R(lua_State*) L, R(const char*) funcName)
{
    lua_getfield(L, SYS_INDEX, funcName);
    
    if (!lua_isfunction(L, -1))
    {
        exception_throw_format(B(zc), ErrorLua, "sys.%s() does not exist; expected it to be defined in " SYS_SCRIPT, funcName);
        lua_pop(L, 1);
    }
}

void zc_lua_init(R(ZC*) zc)
{
    R(lua_State*) L = lua_sys_open(B(zc));
    
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

void zc_lua_clear(R(lua_State*) L)
{
    lua_settop(L, SYS_INDEX);
}

static void zc_lua_push(R(ZC*) zc, R(lua_State*) L, int index, R(const char*) getName)
{
    lua_getfield(L, SYS_INDEX, getName);
    lua_pushinteger(L, index);
    if (!lua_sys_call_no_throw(B(zc), L, 1, 1))
        lua_pushnil(L);
}

static void zc_lua_push_object(R(ZC*) zc, R(lua_State*) L, int index)
{
    zc_lua_push(zc, L, index, "getObject");
}

static void zc_lua_push_callback(R(ZC*) zc, R(lua_State*) L, int index)
{
    zc_lua_push(zc, L, index, "getCallback");
}

static void zc_lua_push_timer(R(ZC*) zc, R(lua_State*) L, int index)
{
    zc_lua_push(zc, L, index, "getTimer");
}

void zc_lua_create_object(R(ZC*) zc, R(lua_State*) L, R(LuaObject*) lobj, R(const char*) funcName)
{
    zc_lua_push_sys_func(zc, L, funcName);
    
    lua_pushlightuserdata(L, lobj);
    lobj->index = (lua_sys_call_no_throw(B(zc), L, 1, 1)) ? lua_tointeger(L, -1) : 0;
    zc_lua_clear(L);
}

void zc_lua_destroy_object(R(ZC*) zc, R(LuaObject*) lobj)
{
    R(lua_State*) L = zc->L;
    
    zc_lua_push_sys_func(zc, L, "objectGC");
    lua_pushinteger(L, lobj->index);
    lua_sys_call_no_throw(B(zc), L, 1, 0);
}

int zc_lua_object_get_index(R(LuaObject*) lobj)
{
    return lobj->index;
}

static void zc_lua_timer_callback(R(Timer*) timer)
{
    R(LuaTimer*) ltimer = timer_userdata_type(timer, LuaTimer);
    R(ZC*) zc           = ltimer->zc;
    R(lua_State*) L     = zc->L;
    
    zc_lua_push_callback(zc, L, ltimer->luaCallback);
    zc_lua_push_timer(zc, L, ltimer->luaObj.index);
    lua_sys_call_no_throw(B(zc), L, 1, 0);
}

LuaTimer* zc_lua_timer_create(R(ZC*) zc, uint32_t periodMilliseconds, int luaCallback, int timerIndex, int start)
{
    R(LuaTimer*) timer = eqp_alloc_type(B(zc), LuaTimer);
    
    timer->luaObj.index = timerIndex;
    timer->luaCallback  = luaCallback;
    timer_init(&timer->timer, &zc->timerPool, periodMilliseconds, zc_lua_timer_callback, timer, start);
    timer->zc           = zc;
    
    return timer;
}

void zc_lua_timer_destroy(R(LuaTimer*) timer)
{
    timer_stop(&timer->timer);
    free(timer);
}

Timer* zc_lua_timer_get_timer(R(LuaTimer*) timer)
{
    return &timer->timer;
}

int zc_lua_timer_get_callback_index(R(LuaTimer*) timer)
{
    return timer->luaCallback;
}

void zc_lua_timer_set_callback_index(R(LuaTimer*) timer, int index)
{
    timer->luaCallback = index;
}

#undef SYS_INDEX
#undef SYS_SCRIPT
#undef ZC_INIT_SCRIPT
