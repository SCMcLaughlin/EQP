
#ifndef EQP_LUA_SYS_H
#define EQP_LUA_SYS_H

#include "define.h"
#include "exception.h"
#include "eqp_log.h"
#include "eqp_string.h"
#include <lua.h>
#include <lualib.h>
#include <luaconf.h>
#include <lauxlib.h>

STRUCT_DECLARE(Basic);

lua_State*  lua_sys_open(Basic* basic);

int         lua_sys_run_file_no_throw(Basic* basic, lua_State* L, const char* path, int numReturns);
void        lua_sys_run_file(Basic* basic, lua_State* L, const char* path, int numReturns); // Throws an exception if the file can't be loaded for whatever reason
int         lua_sys_call_no_throw(Basic* basic, lua_State* L, int numArgs, int numReturns);
void        lua_sys_call(Basic* basic, lua_State* L, int numArgs, int numReturns);

String*     lua_sys_field_to_string(Basic* basic, lua_State* L, int index, const char* field);

void        lua_clear(lua_State* L);

#endif//EQP_LUA_SYS_H
