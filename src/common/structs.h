
#ifndef EQP_STRUCTS_H
#define EQP_STRUCTS_H

#include "define.h"

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
    int     zoneId;
    Loc     loc;
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