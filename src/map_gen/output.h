
#ifndef EQP_OUTPUT_H
#define EQP_OUTPUT_H

#include "define.h"
#include "octree.h"
#include "geometry.h"
#include "aligned.h"
#include "aabb.h"
#include "exception.h"
#include "eqp_alloc.h"
#include <zlib.h>

#define EQP_MAP_GEN_PATH_ENV_VARIABLE   "EQP_PATH"
#define EQP_MAP_GEN_MAP_FILE_SIGNATURE  0x4d505145  /* "EQPM" */

STRUCT_DEFINE(MapFileHeader)
{
    uint32_t    signature;
    uint32_t    version;
    float       minZ;
    uint32_t    boxCount;
};

STRUCT_DEFINE(MapFileBox)
{
    Vertex      center;
    Vertex      halfExtent;
    uint32_t    triangleCount;
};

void    output_to_file(Octree* octree, const char* zoneShortName, float minZ);

#endif//EQP_OUTPUT_H
