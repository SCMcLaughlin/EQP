
local ffi = require "ffi"

ffi.cdef[[
void    zc_lua_object_update_index(void* lobj, int index);
]]

return ffi.C
