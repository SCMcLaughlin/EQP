
local ffi = require "ffi"

ffi.cdef[[
typedef struct Zone Zone;

int         zone_get_source_id(Zone* zone);
uint16_t    zone_get_zone_id(Zone* zone);
uint16_t    zone_get_instance_id(Zone* zone);
const char* zone_get_short_name(Zone* zone);
const char* zone_get_long_name(Zone* zone);
uint8_t     zone_get_zone_type_id(Zone* zone);
uint16_t    zone_get_sky_id(Zone* zone);
float       zone_get_gravity(Zone* zone);
float       zone_get_min_clipping_distance(Zone* zone);
float       zone_get_max_clipping_distance(Zone* zone);
float       zone_get_safe_spot_x(Zone* zone);
float       zone_get_safe_spot_y(Zone* zone);
float       zone_get_safe_spot_z(Zone* zone);
float       zone_get_safe_spot_heading(Zone* zone);

void        zone_set_zone_type_id(Zone* zone, uint32_t id);
void        zone_set_sky_id(Zone* zone, uint32_t id);
void        zone_set_gravity(Zone* zone, float value);
void        zone_set_min_clipping_distance(Zone* zone, float value);
void        zone_set_max_clipping_distance(Zone* zone, float value);
void        zone_set_safe_spot(Zone* zone, float x, float y, float z, float heading);

int         zone_are_points_in_line_of_sight(ZC* zc, Zone* zone, float x1, float y1, float z1, float x2, float y2, float z2);
float       zone_get_best_z_for_loc(Zone* zone, float x, float y, float z);
]]

return ffi.C
