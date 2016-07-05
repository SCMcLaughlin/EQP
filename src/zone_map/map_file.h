
#ifndef EQP_MAP_FILE_H
#define EQP_MAP_FILE_H

#include "define.h"
#include "geometry.h"
#include "aabb.h"

#define EQP_MAP_GEN_MAP_FILE_SIGNATURE  0x4d505145  /* "EQPM" */
#define EQP_ZSLICE_COUNT 65536

STRUCT_DEFINE(MapFileHeader)
{
    uint32_t    signature;
    uint32_t    version;
    uint32_t    inflatedLength;
    float       minX;
    float       minY;
    float       minZ;
    float       incrementX;
    float       incrementY;
    uint32_t    triangleCount;
    uint32_t    nodeCount;
    uint32_t    zSliceTriangleCount;
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

STRUCT_DEFINE(MapFileZSlice)
{
    uint32_t    triangleCount;
    uint32_t    triangleIndex;
};

#endif//EQP_MAP_FILE_H
