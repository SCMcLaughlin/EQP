
#ifndef EQP_STRUCTS_H
#define EQP_STRUCTS_H

#include "define.h"

#define EQP_CLIENT_BIND_POINT_COUNT 5

STRUCT_DEFINE(Coin)
{
    uint32_t    pp;
    uint32_t    gp;
    uint32_t    sp;
    uint32_t    cp;
};

STRUCT_DEFINE(Loc)
{
    float x;
    float y;
    float z;
    float heading;
};

STRUCT_DEFINE(BindPoint)
{
    uint16_t    zoneId;
    uint16_t    instanceId;
    Loc         loc;
};

STRUCT_DEFINE(FogStats)
{
    uint8_t     red;
    uint8_t     green;
    uint8_t     blue;
    float       minClippingDistance;
    float       maxClippingDistance;
};

#endif//EQP_STRUCTS_H
