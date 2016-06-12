
#ifndef EQP_PROTOCOL_OPCODES_H
#define EQP_PROTOCOL_OPCODES_H

#include "define.h"

ENUM_DEFINE(ProtocolOpcode)
{
    ProtocolOp_None                 = 0x0000,
    ProtocolOp_SessionRequest       = 0x0001,
    ProtocolOp_SessionResponse      = 0x0002,
    ProtocolOp_Combined             = 0x0003,
    ProtocolOp_SessionDisconnect    = 0x0005,
    ProtocolOp_KeepAlive            = 0x0006,
    ProtocolOp_SessionStatsRequest  = 0x0007,
    ProtocolOp_SessionStatsResponse = 0x0008,
    ProtocolOp_Packet               = 0x0009,
    ProtocolOp_Fragment             = 0x000d,
    ProtocolOp_OutOfOrder           = 0x0011,
    ProtocolOp_Ack                  = 0x0015,
    ProtocolOp_AppCombined          = 0x0019
};

#endif//EQP_PROTOCOL_OPCODES_H
