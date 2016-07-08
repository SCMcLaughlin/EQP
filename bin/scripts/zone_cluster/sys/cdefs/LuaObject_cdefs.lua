
local ffi = require "ffi"

ffi.cdef[[
void    zc_lua_object_set_index(void* lobj, int index);
int     zc_lua_object_get_index(void* lobj);
]]

return ffi.C
