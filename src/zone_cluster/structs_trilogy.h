
#ifndef EQP_STRUCTS_TRILOGY_H
#define EQP_STRUCTS_TRILOGY_H

#include "define.h"

STRUCT_DEFINE(Trilogy_ZoneEntry)
{
    uint32_t    checksum;
    char        characterName[32];
};

#endif//EQP_STRUCTS_TRILOGY_H
