
#ifndef EQP_TCP_PACKET_STRUCTS_H
#define EQP_TCP_PACKET_STRUCTS_H

#include "define.h"

ENUM_DEFINE(TcpOp)
{
    TcpOp_KeepAlive         = 0x0001,
    TcpOp_LoginServerInfo   = 0x1000,
    TcpOp_LoginServerStatus = 0x1001,
    TcpOp_NewLoginServer    = 0x1008,
};

#pragma pack(1)

STRUCT_DEFINE(TcpPacketHeader)
{
    uint16_t opcode;
    uint16_t length;
};

STRUCT_DEFINE(Tcp_NewLoginServer)
{
    char    longName[201];          // name the worldserver wants
    char    shortName[50];          // shortname the worldserver wants
    char    remoteAddress[125];     // DNS address of the server
    char    localAddress[125];      // DNS address of the server
    char    account[31];            // account name for the worldserver
    char    password[31];           // password for the name
    char    protocolversion[25];    // Major protocol version number
    char    serverversion[64];      // minor server software version number
    uint8_t servertype;             // 0=world, 1=chat, 2=login, 3=MeshLogin
};

STRUCT_DEFINE(Tcp_LoginServerStatus)
{
    int status;
    int playerCount;
    int zoneCount;
};

#pragma pack()

#endif//EQP_TCP_PACKET_STRUCTS_H
