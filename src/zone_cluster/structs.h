
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

STRUCT_DEFINE(BindPoint)
{
    int     zoneId;
    float   x;
    float   y;
    float   z;
    float   heading;
};

#endif//EQP_STRUCTS_H
