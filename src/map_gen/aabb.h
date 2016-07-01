
#ifndef EQP_AABB_H
#define EQP_AABB_H

#include "define.h"
#include "geometry.h"

STRUCT_DEFINE(AABB)
{
    Vertex  minCorner;
    Vertex  maxCorner;
};

void    aabb_init(AABB* box);
void    aabb_init_from_corners(AABB* box, Vertex* corners, Vertex* center);
void    aabb_add_internal_point(AABB* box, float x, float y, float z);

Vertex  aabb_get_center(AABB* box);
Vertex  aabb_get_half_extent(AABB* box);

int     aabb_contains_point(AABB* box, Vertex* v);

void    aabb_get_all_corners_and_center(AABB* box, Vertex* out, Vertex* outCenter);

#endif//EQP_AABB_H
