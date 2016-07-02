
#ifndef EQP_WLD_FRAGMENTS_H
#define EQP_WLD_FRAGMENTS_H

#include "define.h"

#define EQP_WLD_FRAG_HEADER_LENGTH 8
#define EQP_WLD_TRIANGLE_FLAG_PERMEABLE 0x10

#pragma pack(1)

STRUCT_DEFINE(Fragment)
{
    uint32_t    length;
    uint32_t    type;
    int         nameRef;
};

#pragma pack()

STRUCT_DEFINE(Frag36)
{
    Fragment    header;
    uint32_t    flag;
    int         materialListRef;
    int         animVertRef;
    int         unknownA[2];
    float       x, y, z;
    float       rotX, rotY, rotZ;
    float       maxDist;
    float       minX, minY, minZ;
    float       maxX, maxY, maxZ;
    uint16_t    vertCount;
    uint16_t    uvCount;
    uint16_t    normalCount;
    uint16_t    colorCount;
    uint16_t    polyCount;
    uint16_t    boneAssignCount;
    uint16_t    polyTextureCount;
    uint16_t    vertTextureCount;
    uint16_t    size9;
    uint16_t    scale;
};

STRUCT_DEFINE(Frag15)
{
    Fragment    header;
    int         modelNameRef;
    uint32_t    flag;
    int         refB;
    float       x, y, z;
    float       rotX, rotY, rotZ;
    float       scaleX, scaleY, scaleZ;
    int         refC;
    uint32_t    refCParam;
};

#pragma pack(1)

STRUCT_DEFINE(Wld_Vertex)
{
    int16_t x, y, z;
};

STRUCT_DEFINE(Wld_Uv16)
{
    int16_t u, v;
};

STRUCT_DEFINE(Wld_Uv32)
{
    float u, v;
};

STRUCT_DEFINE(Wld_Normal)
{
    int8_t i, j, k;
};

STRUCT_DEFINE(Wld_Color)
{
    uint8_t r, g, b, a;
};

STRUCT_DEFINE(Wld_Triangle)
{
    uint16_t flag;
    uint16_t index[3];
};

STRUCT_DEFINE(Wld_TextureEntry)
{
    uint16_t count;
    uint16_t index;
};

STRUCT_DEFINE(Wld_BoneAssignment)
{
    uint16_t count;
    uint16_t index;
};

#pragma pack()

#endif//EQP_WLD_FRAGMENTS_H
