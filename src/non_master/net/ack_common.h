
#ifndef EQP_ACK_COMMON_H
#define EQP_ACK_COMMON_H

#include "define.h"

#define EQP_ACK_WINDOW_HALF_SIZE 1024

ENUM_DEFINE(AckRelativity)
{
    AckPresent,
    AckFuture,
    AckPast
};

AckRelativity ack_compare(uint16_t got, uint16_t expected);

#endif//EQP_ACK_COMMON_H
