
local ffi = require "ffi"

require "ZoneCluster_cdefs"

ffi.cdef[[
typedef struct LuaTimer LuaTimer;
typedef struct Timer Timer;

LuaTimer*   zc_lua_timer_create(ZC* zc, uint32_t periodMilliseconds, int luaCallback, int timerIndex, int start);
void        zc_lua_timer_destroy(LuaTimer* timer);
Timer*      zc_lua_timer_get_timer(LuaTimer* timer);
int         zc_lua_timer_get_callback_index(LuaTimer* timer);
void        timer_stop(Timer* timer);
void        timer_restart(Timer* timer);
void        timer_delay(Timer* timer, uint32_t milliseconds);
void        timer_force_trigger_on_next_cycle(Timer* timer);
void        timer_execute_callback(Timer* timer);
]]

return ffi.C
