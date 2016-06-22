
#include "lua_object.h"
#include "zone_cluster.h"

#define SYS_INDEX       2
#define SYS_SCRIPT      "scripts/zone_cluster/sys/sys.lua"
#define ZC_INIT_SCRIPT  "scripts/zone_cluster/init/zone_cluster_init.lua"

void zc_lua_init(R(ZC*) zc)
{
    R(lua_State*) L = lua_sys_open(B(zc));
    
    zc->L = L;
    
    // Load sys.lua and keep its return value at SYS_INDEX on the stack
    lua_sys_run_file(B(zc), L, SYS_SCRIPT, 1);
    
    // Create the Lua-side ZC object
    lua_getfield(L, SYS_INDEX, "createZoneCluster");
    if (!lua_isfunction(L, -1))
        exception_throw_literal(B(zc), ErrorLua, "sys.createZoneCluster() does not exist; expected it to be defined in " SYS_SCRIPT);
    lua_pushlightuserdata(L, zc);
    lua_sys_call(B(zc), L, 1, 0);
    
    // Run the zone cluster init script
    lua_sys_run_file_no_throw(B(zc), L, ZC_INIT_SCRIPT, 0);
}

void zc_lua_clear(R(lua_State*) L)
{
    lua_settop(L, SYS_INDEX);
}

void zc_lua_create_object(R(ZC*) zc, R(lua_State*) L, R(LuaObject*) lobj, R(const char*) funcName)
{
    lua_getfield(L, SYS_INDEX, funcName);
    
    if (!lua_isfunction(L, -1))
        exception_throw_format(B(zc), ErrorLua, "sys.%s() does not exist; expected it to be defined in " SYS_SCRIPT, funcName);
    
    lua_pushlightuserdata(L, lobj);
    lua_sys_call(B(zc), L, 1, 1);
    
    lobj->index = lua_tointeger(L, -1);
    
    zc_lua_clear(L);
}

void zc_lua_object_update_index(R(LuaObject*) lobj, int index)
{
    lobj->index = index;
}

#undef SYS_INDEX
#undef SYS_SCRIPT
#undef ZC_INIT_SCRIPT
