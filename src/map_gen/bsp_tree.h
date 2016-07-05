
#ifndef EQP_BSP_TREE_H
#define EQP_BSP_TREE_H

#include "define.h"
#include "geometry.h"
#include "eqp_array.h"
#include "eqp_alloc.h"
#include "map_file.h"
#include "z_slices.h"
#include "aabb.h"

#define EQP_BSP_MAX_TRIANGLES_PER_NODE 4

STRUCT_DEFINE(BspNode)
{
    uint32_t    triangleCount;
    uint32_t    leftIndex;
    uint32_t    rightIndex;
    AABB        bounds;
    Triangle*   triangles;
};

STRUCT_DEFINE(BspTree)
{
    Array*      nodes;
    Array*      stack;
    Basic*      basic;
    uint32_t    triangleCount;
};

void    bsp_tree_init(Basic* basic, BspTree* bsp);
void    bsp_tree_deinit(BspTree* bsp);
void    bsp_tree_generate(BspTree* bsp, Array* vertices, ZSlices* zSlices);

#endif//EQP_BSP_TREE_H
