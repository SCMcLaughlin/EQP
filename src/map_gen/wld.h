
#ifndef EQP_WLD_H
#define EQP_WLD_H

#include "define.h"
#include "eqp_array.h"
#include "eqp_hash_table.h"
#include "eqp_alloc.h"
#include "wld_fragments.h"

STRUCT_DECLARE(MapGen);

STRUCT_DEFINE(Wld)
{
    HashTable*  objectDefsByName;
    char*       stringBlock;
    uint32_t    stringsLength;
    uint32_t    version;
    byte*       data;
    uint32_t    length;
    float       minZ;
    Wld*        objDefs;
};

void    wld_read_zone_vertices(MapGen* map, byte* data, uint32_t len, float* minZ);
Wld*    wld_read_object_definitions(MapGen* map, byte* data, uint32_t len);
void    wld_read_object_placements(MapGen* map, Wld* objDefs, byte* data, uint32_t len);

void    wld_destroy(Wld* wld);

#endif//EQP_WLD_H
