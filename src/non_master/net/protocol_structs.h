
#ifndef EQP_PROTOCOL_STRUCTS_H
#define EQP_PROTOCOL_STRUCTS_H

#include "define.h"

#pragma pack(1)

STRUCT_DEFINE(Protocol_SessionRequest)
{
    uint32_t    unknown;
    uint32_t    sessionId;
    uint32_t    maxLength;
};

ENUM_DEFINE(ProtocolValidation)
{
    ProtocolValidation_None = 0x00,
    ProtocolValidation_Crc  = 0x02
};

ENUM_DEFINE(ProtocolFormat)
{
    ProtocolFormat_None         = 0x00,
    ProtocolFormat_Compressed   = 0x01,
    ProtocolFormat_Encoded      = 0x04
};

STRUCT_DEFINE(Protocol_SessionResponse)
{
    uint16_t    opcode;
    uint32_t    sessionId;
    uint32_t    crcKey;
    uint8_t     validation; // From ProtocolValidation above
    uint8_t     format;     // From ProtocolFormat above
    uint8_t     unknownA;
    uint32_t    maxLength;
    uint32_t    unknownB;
};

STRUCT_DEFINE(Protocol_SessionStatsClient)
{
    uint16_t    requestId;
    uint32_t    lastLocalDelta;
    uint32_t    averageDelta;
    uint32_t    lowDelta;
    uint32_t    highDelta;
    uint32_t    lastRemoteDelta;
    uint64_t    packetsSent;
    uint64_t    packetsReceived;
};

STRUCT_DEFINE(Protocol_SessionStatsServer)
{
    uint16_t    opcode;
    uint16_t    requestId;
    uint32_t    serverTime;
    uint64_t    packetsSentEcho;
    uint64_t    packetsReceivedEcho;
    uint64_t    packetsSent;
    uint64_t    packetsReceived;
};

STRUCT_DEFINE(Protocol_SessionDisconnect)
{
    uint16_t    opcode;
    uint32_t    sessionId;
};

#pragma pack()

#endif//EQP_PROTOCOL_STRUCTS_H
