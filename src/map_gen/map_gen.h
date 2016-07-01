
#ifndef EQP_MAP_GEN_H
#define EQP_MAP_GEN_H

#include "define.h"
#include "eqp_basic.h"
#include "eqp_clock.h"
#include "eqp_array.h"
#include "exception.h"
#include "pfs.h"
#include "wld.h"
#include "matrix.h"
#include "geometry.h"
#include "octree.h"

STRUCT_DEFINE(MapGen)
{
    Basic   basic;
    Array*  vertices;
    Octree  octree;
};

void    map_gen_init(MapGen* map);
void    map_gen_deinit(MapGen* map);
void    map_gen_reset(MapGen* map);

void    map_gen_read_vertices(MapGen* map, const char* path);
void    map_gen_add_object_placement(MapGen* map, Array* object, Matrix* matrix);

#endif//EQP_MAP_GEN_H
