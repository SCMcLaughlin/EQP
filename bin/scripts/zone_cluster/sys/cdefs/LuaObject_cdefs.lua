
local ffi = require "ffi"

ffi.cdef[[
int zc_lua_object_get_index(void* lobj);
]]

return ffi.C
