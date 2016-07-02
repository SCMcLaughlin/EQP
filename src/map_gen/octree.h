
#ifndef EQP_OCTREE_H
#define EQP_OCTREE_H

#include "define.h"
#include "eqp_array.h"
#include "aabb.h"
#include "geometry.h"

STRUCT_DECLARE(Basic);

STRUCT_DEFINE(OctreeNode)
{
    Array*  triangles;
    AABB    box;
};

STRUCT_DEFINE(Octree)
{
    Array*      nodes;
    uint32_t    triangleCount;
    Basic*      basic;
};

void    octree_init(Basic* basic, Octree* octree);
void    octree_deinit(Octree* octree);
void    octree_generate(Octree* octree, Array* vertices, uint32_t maxTrianglesPerNode);

#endif//EQP_OCTREE_H
