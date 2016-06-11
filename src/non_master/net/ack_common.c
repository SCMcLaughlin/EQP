
#include "ack_common.h"

AckRelativity ack_compare(uint16_t got, uint16_t expected)
{
    if (got == expected)
        return AckPresent;
    
    if ((got > expected && got < expected + EQP_ACK_WINDOW_HALF_SIZE) || got < (expected - EQP_ACK_WINDOW_HALF_SIZE))
        return AckFuture;

    return AckPast;
}
