
#ifndef EQP_GEOMETRY_H
#define EQP_GEOMETRY_H

#include "define.h"

#pragma pack(4)

STRUCT_DEFINE(Vertex)
{
    float x, y, z;
};

STRUCT_DEFINE(Triangle)
{
    Vertex points[3];
};

#pragma pack()

#endif//EQP_GEOMETRY_H
