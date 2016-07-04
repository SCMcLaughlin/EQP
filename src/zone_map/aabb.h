
#ifndef EQP_AABB_H
#define EQP_AABB_H

#include "define.h"
#include "geometry.h"

ENUM_DEFINE(AABB_Axis)
{
    AABB_AxisX,
    AABB_AxisY,
    AABB_AxisZ,
};

ENUM_DEFINE(AABB_Clip)
{
    AABB_Clip_MinX = (1 << 0),
    AABB_Clip_MaxX = (1 << 1),
    AABB_Clip_MinY = (1 << 2),
    AABB_Clip_MaxY = (1 << 3),
    AABB_Clip_MinZ = (1 << 4),
    AABB_Clip_MaxZ = (1 << 5),
};

STRUCT_DEFINE(AABB)
{
    Vector  minCorner;
    Vector  maxCorner;
};

void        aabb_init(AABB* box);
void        aabb_init_from_corners(AABB* box, Vector* a, Vector* b);
void        aabb_add_internal_point(AABB* box, float x, float y, float z);
void        aabb_add_internal_point_vector(AABB* box, Vector* vec);

Vector      aabb_get_center(AABB* box);
Vector      aabb_get_extent(AABB* box);
Vector      aabb_get_half_extent(AABB* box);

int         aabb_contains_point(AABB* box, Vector* v);
uint32_t    aabb_clip_vector(AABB* box, Vector* v);
uint32_t    aabb_clip_triangle(AABB* box, Triangle* tri, uint32_t* andCode);
int         aabb_contains_triangle_precise(AABB* box, Triangle* tri, uint32_t* orCode);

int         aabb_intersected_by_ray(AABB* box, Vector* origin, Vector* direction, Vector* intersect);
int         aabb_intersected_by_line_segment(AABB* box, Vector* origin, Vector* direction, float* distance);

void        aabb_clamp(AABB* box, AABB* to);
void        aabb_split(AABB* box, int axisId, AABB* b1, AABB* b2);
void        aabb_get_all_corners_and_center(AABB* box, Vector* out, Vector* outCenter);

#endif//EQP_AABB_H
