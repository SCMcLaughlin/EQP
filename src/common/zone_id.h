
#ifndef EQP_ZONE_ID_H
#define EQP_ZONE_ID_H

#include "define.h"
#include "eqp_hash_table.h"

STRUCT_DECLARE(Basic);

typedef HashTable ZoneShortNameMap;

ZoneShortNameMap*   zone_short_name_map_create(R(Basic*) basic);
#define             zone_short_name_map_destroy(map) hash_table_destroy((map))
int                 zone_id_by_short_name(R(ZoneShortNameMap*) map, R(const char*) shortName, uint32_t len);

const char* zone_short_name_by_id(int id);
const char* zone_long_name_by_id(int id);

#endif//EQP_ZONE_ID_H
