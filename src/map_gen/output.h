
#ifndef EQP_OUTPUT_H
#define EQP_OUTPUT_H

#include "define.h"
#include "octree.h"
#include "geometry.h"
#include <zlib.h>

STRUCT_DEFINE(MapFileHeader)
{
    uint32_t    signature;
    uint32_t    version;
    uint32_t    boxCount;
};

STRUCT_DEFINE(MapFileBox)
{
    Vertex      center;
    Vertex      halfExtent;
    uint32_t    triangleCount;
};

void    output_to_file(Octree* octree, const char* fileName, const char* dirPath);

#endif//EQP_OUTPUT_H
