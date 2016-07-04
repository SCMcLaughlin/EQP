
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
#include "bsp_tree.h"
#include "output.h"

#ifdef EQP_LINUX
# include <dirent.h>
#endif

#define EQP_MAP_GEN_DEFAULT_TRIANGLES_PER_OCTREE_NODE 512

STRUCT_DEFINE(MapGen)
{
    Basic   basic;
    Array*  vertices;
    BspTree bsp;
    //Octree  octree;
};

void    map_gen_init(MapGen* map);
void    map_gen_deinit(MapGen* map);
void    map_gen_reset(MapGen* map);

void    map_gen_read_all_zones(MapGen* map, const char* dirPath);
void    map_gen_read_single_zone(MapGen* map, const char* dirPath, const char* fileName);
void    map_gen_read_vertices(MapGen* map, const char* path);
void    map_gen_add_object_placement(MapGen* map, Array* object, Matrix* matrix);

#endif//EQP_MAP_GEN_H
