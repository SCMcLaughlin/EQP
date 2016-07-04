
#ifndef EQP_MAP_FILE_H
#define EQP_MAP_FILE_H

#include "define.h"
#include "geometry.h"
#include "aabb.h"

#define EQP_MAP_GEN_MAP_FILE_SIGNATURE  0x4d505145  /* "EQPM" */

STRUCT_DEFINE(MapFileHeader)
{
    uint32_t    signature;
    uint32_t    version;
    uint32_t    inflatedLength;
    float       minZ;
    uint32_t    triangleCount;
    uint32_t    nodeCount;
};

#if 0
STRUCT_DEFINE(MapFileOctreeBox)
{
    Vertex      center;
    Vertex      halfExtent;
    uint32_t    triangleCount;
};
#endif

STRUCT_DEFINE(MapFileBspNode)
{
    uint32_t    triangleCount;
    uint32_t    leftIndex;
    uint32_t    rightIndex;
    AABB        bounds;
    uint32_t    trianglesIndex;
};

#endif//EQP_MAP_FILE_H
