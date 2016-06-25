
local ffi = require "ffi"

ffi.cdef[[
typedef struct ZoneCluster ZoneCluster;

void            zone_cluster_set_max_zones(ZoneCluster* zc, uint16_t maxZones);

void            zc_mgr_set_max_zones_per_cluster(void* mgr, uint16_t maxPer);
ZoneCluster*    zc_mgr_start_zone_cluster(void* mgr);
void            zc_mgr_add_zone_reservation(void* mgr, ZoneCluster* zc, const char* shortName, int zoneId, int instanceId, int alwaysUp);
]]

return ffi.C
