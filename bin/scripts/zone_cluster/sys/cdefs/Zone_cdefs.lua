
local ffi = require "ffi"

ffi.cdef[[
typedef struct Zone Zone;

int         zone_get_source_id(Zone* zone);
uint16_t    zone_get_zone_id(Zone* zone);
uint16_t    zone_get_instance_id(Zone* zone);
const char* zone_get_short_name(Zone* zone);
const char* zone_get_long_name(Zone* zone);
]]

return ffi.C
