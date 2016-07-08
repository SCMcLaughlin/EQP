
local ffi = require "ffi"

require "ZoneCluster_cdefs"

ffi.cdef[[
typedef struct Npc Npc;

uint32_t    npc_adhoc_id(Npc* npc);
]]

return ffi.C
