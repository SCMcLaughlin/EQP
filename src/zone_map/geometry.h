
#ifndef EQP_GEOMETRY_H
#define EQP_GEOMETRY_H

#include "define.h"

#define FLOAT_EPSILON   0.00001f

#pragma pack(4)

STRUCT_DEFINE(Vertex)
{
    float x, y, z;
};

STRUCT_DEFINE(Triangle)
{
    Vertex points[3];
};

typedef Vertex Vector;

#pragma pack()

#define vector_set(v, ix, iy, iz)   \
    (v)->x = (ix);                  \
    (v)->y = (iy);                  \
    (v)->z = (iz)

#define vector_dot_product(a, b) ((a)->x * (b)->x + (a)->y * (b)->y + (a)->z * (b)->z)

#define vector_cross_product(a, b, c)               \
    (a)->x = (b)->y * (c)->z - (b)->z * (c)->y;     \
    (a)->y = (b)->z * (c)->x - (b)->x * (c)->z;     \
    (a)->z = (b)->x * (c)->y - (b)->y * (c)->x

#define vector_difference(a, b, c)  \
    (a)->x = (b)->x - (c)->x;       \
    (a)->y = (b)->y - (c)->y;       \
    (a)->z = (b)->z - (c)->z

#define vector_length_squared(v) ((v)->x * (v)->x + (v)->y * (v)->y + (v)->z * (v)->z)

STRUCT_DEFINE(Vector64)
{
    double x, y, z;
};

/*
STRUCT_DEFINE(Triangle64)
{
    Vector64 points[3];
    Vector64 normal;
};
*/

typedef Triangle Triangle64;

#endif//EQP_GEOMETRY_H
