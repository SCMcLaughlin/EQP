
#ifndef EQP_MAP_FILE_H
#define EQP_MAP_FILE_H

#include "define.h"
#include "geometry.h"

#define EQP_MAP_GEN_MAP_FILE_SIGNATURE  0x4d505145  /* "EQPM" */

STRUCT_DEFINE(MapFileHeader)
{
    uint32_t    signature;
    uint32_t    version;
    uint32_t    inflatedLength;
    float       minZ;
    uint32_t    triangleCount;
    uint32_t    boxCount;
};

STRUCT_DEFINE(MapFileBox)
{
    Vertex      center;
    Vertex      halfExtent;
    uint32_t    triangleCount;
};

#endif//EQP_MAP_FILE_H
