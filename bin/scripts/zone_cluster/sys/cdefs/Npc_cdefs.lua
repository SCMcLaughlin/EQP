
local ffi = require "ffi"

require "ZoneCluster_cdefs"

ffi.cdef[[
typedef struct Npc Npc;
]]

return ffi.C
