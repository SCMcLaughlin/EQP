
#ifndef EQP_PACKET_INPUT_H
#define EQP_PACKET_INPUT_H

#include "define.h"

STRUCT_DEFINE(PacketInput)
{
    uint32_t    length;
    uint16_t    opcode;
    byte        data[0];
};

#endif//EQP_PACKET_INPUT_H
