
#ifndef EQP_Z_SLICES_H
#define EQP_Z_SLICES_H

#include "define.h"
#include "geometry.h"
#include "aabb.h"
#include "eqp_alloc.h"
#include "eqp_array.h"
#include "map_file.h"

STRUCT_DEFINE(Basic);

STRUCT_DEFINE(ZSlice)
{
    uint32_t    triangleCount;
    uint32_t    triangleIndex;
};

STRUCT_DEFINE(ZSlices)
{
    float   minX;
    float   minY;
    float   incrementX;
    float   incrementY;
    ZSlice* slices;
    Array*  triangles;
};

void    zslices_init(Basic* basic, ZSlices* slices, AABB* box, Triangle* triangles, uint32_t triCount);
void    zslices_deinit(ZSlices* slices);

#endif//EQP_Z_SLICES_H
