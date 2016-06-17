
#ifndef EQP_TCP_PACKET_STRUCTS_H
#define EQP_TCP_PACKET_STRUCTS_H

#include "define.h"

ENUM_DEFINE(TcpOp)
{
    TcpOp_KeepAlive             = 0x0001,
    TcpOp_LoginServerInfo       = 0x1000,
    TcpOp_LoginServerStatus     = 0x1001,
    TcpOp_ClientLoginAuth       = 0x1002,
    TcpOp_NewLoginServer        = 0x1008,
    TcpOp_ClientLoginRequest    = 0xab00,
    TcpOp_ClientLoginResponse   = 0xab01
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

STRUCT_DEFINE(Tcp_NewLoginServerSend)
{
    TcpPacketHeader     header;
    Tcp_NewLoginServer  data;
};

STRUCT_DEFINE(Tcp_LoginServerStatus)
{
    int status;
    int playerCount;
    int zoneCount;
};

STRUCT_DEFINE(Tcp_LoginServerStatusSend)
{
    TcpPacketHeader         header;
    Tcp_LoginServerStatus   data;
};

STRUCT_DEFINE(Tcp_ClientLoginRequest)
{
    uint32_t    accountId;
    uint32_t    serverId;
    uint32_t    unused[2];
};

STRUCT_DEFINE(Tcp_ClientLoginRequestSend)
{
    TcpPacketHeader         header;
    Tcp_ClientLoginRequest  data;
};

STRUCT_DEFINE(Tcp_ClientLoginResponse)
{
    uint32_t    accountId;
    uint32_t    serverId;
    int8_t      response;   // -3 = World Full, -2 = Banned, -1 = Suspended, 0 = Denied, 1 = Allowed
    uint32_t    unused[2];
};

STRUCT_DEFINE(Tcp_ClientLoginResponseSend)
{
    TcpPacketHeader         header;
    Tcp_ClientLoginResponse data;
};

STRUCT_DEFINE(Tcp_ClientLoginAuth)
{
    TcpPacketHeader header;
    uint32_t        accountId;          // ID# in login server's db
    char            accountName[30];    // username in login server's db
    char            sessionKey[30];     // the Key the client will present
    uint8_t         loginAdmin;         // login server admin level
    int16_t         serverAdmin;        // login's suggested worldadmin level setting for this user, up to the world if they want to obey it
    uint32_t        ip;
    uint8_t         isLocal;            // 1 if the client is from the local network
};

#pragma pack()

#endif//EQP_TCP_PACKET_STRUCTS_H
