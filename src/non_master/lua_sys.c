
#include "lua_sys.h"
#include "eqp_basic.h"

#define SCRIPT_INCLUDE  "scripts/include.lua"
#define TRACEBACK_INDEX 1

lua_State* lua_sys_open(R(Basic*) basic)
{
    R(lua_State*) L = luaL_newstate();
    
    if (L == NULL)
        exception_throw_literal(basic, ErrorLua, "[lua_sys_open] luaL_newstate() failed");
    
    luaL_openlibs(L);
    
    // debug.traceback is kept at index 1 on the lua stack at all times
    lua_getglobal(L, "debug");
    lua_getfield(L, TRACEBACK_INDEX, "traceback");
    lua_replace(L, TRACEBACK_INDEX);
    
    lua_sys_run_file(basic, L, SCRIPT_INCLUDE, 0);
    
    return L;
}

int lua_sys_run_file_no_throw(R(Basic*) basic, R(lua_State*) L, R(const char*) path, int numReturns)
{
    int ret = true;
    
    if (luaL_loadfile(L, path) || lua_pcall(L, 0, numReturns, TRACEBACK_INDEX))
    {
        log_format(basic, LogLua, "[lua_sys_run_file_no_throw] %s", lua_tostring(L, -1));
        lua_clear(L);
        ret = false;
    }
    
    return ret;
}

void lua_sys_run_file(R(Basic*) basic, R(lua_State*) L, R(const char*) path, int numReturns)
{
    if (!lua_sys_run_file_no_throw(basic, L, path, numReturns))
        exception_throw(basic, ErrorLua);
}

String* lua_sys_field_to_string(R(Basic*) basic, R(lua_State*) L, int index, R(const char*) field)
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

void lua_clear(R(lua_State*) L)
{
    lua_settop(L, TRACEBACK_INDEX);
}

#undef SCRIPT_INCLUDE
#undef TRACEBACK_INDEX
