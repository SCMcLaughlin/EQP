
#include "lua_sys.h"
#include "eqp_basic.h"

#define SCRIPT_INCLUDE  "scripts/include.lua"
#define TRACEBACK_INDEX 1

lua_State* lua_sys_open(Basic* basic)
{
    lua_State* L = luaL_newstate();
    
    if (L == NULL)
        exception_throw_literal(basic, ErrorLua, "[lua_sys_open] luaL_newstate() failed");
    
    luaL_openlibs(L);
    
    // debug.traceback is kept at index 1 on the lua stack at all times
    lua_getglobal(L, "debug");
    lua_getfield(L, TRACEBACK_INDEX, "traceback");
    lua_replace(L, TRACEBACK_INDEX);
    
    // Seed the RNG
    lua_getglobal(L, "math");
    lua_getfield(L, -1, "randomseed");
    lua_pushinteger(L, clock_microseconds());
    lua_sys_call(basic, L, 1, 0);
    lua_pop(L, 1); // Pop math
    
    // Run the universal include file
    lua_sys_run_file(basic, L, SCRIPT_INCLUDE, 0);
    
    return L;
}

int lua_sys_run_file_no_throw(Basic* basic, lua_State* L, const char* path, int numReturns)
{
    int ret = true;
    
    if (luaL_loadfile(L, path) || lua_pcall(L, 0, numReturns, TRACEBACK_INDEX))
    {
        log_format(basic, LogLua, "[lua_sys_run_file_no_throw] %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        ret = false;
    }
    
    return ret;
}

void lua_sys_run_file(Basic* basic, lua_State* L, const char* path, int numReturns)
{
    if (luaL_loadfile(L, path) || lua_pcall(L, 0, numReturns, TRACEBACK_INDEX))
    {
        log_format(basic, LogLua, "[lua_sys_run_file] %s", lua_tostring(L, -1));
        exception_set_message(basic, lua_tostring(L, -1), lua_objlen(L, -1));
        lua_pop(L, 1);
        exception_throw(basic, ErrorLua);
    }
}

int lua_sys_call_no_throw(Basic* basic, lua_State* L, int numArgs, int numReturns)
{
    int ret = true;
    
    if (lua_pcall(L, numArgs, numReturns, TRACEBACK_INDEX))
    {
        log_format(basic, LogLua, "[lua_sys_call_no_throw] %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        ret = false;
    }
    
    return ret;
}

void lua_sys_call(Basic* basic, lua_State* L, int numArgs, int numReturns)
{
    if (lua_pcall(L, numArgs, numReturns, TRACEBACK_INDEX))
    {
        log_format(basic, LogLua, "[lua_sys_call] %s", lua_tostring(L, -1));
        exception_set_message(basic, lua_tostring(L, -1), lua_objlen(L, -1));
        lua_pop(L, 1);
        exception_throw(basic, ErrorLua);
    }
}

String* lua_sys_field_to_string(Basic* basic, lua_State* L, int index, const char* field)
{
    String* str;
    
    lua_getfield(L, index, field);
    
    if (lua_isstring(L, -1))
        str = string_create_from_cstr(basic, lua_tostring(L, -1), lua_objlen(L, -1));
    else
        str = string_create(basic);
    
    lua_pop(L, 1);
    
    return str;
}

void lua_clear(lua_State* L)
{
    lua_settop(L, TRACEBACK_INDEX);
}

#undef SCRIPT_INCLUDE
#undef TRACEBACK_INDEX
