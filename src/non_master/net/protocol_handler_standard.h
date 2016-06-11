
#ifndef EQP_PROTOCOL_HANDLER_STANDARD_H
#define EQP_PROTOCOL_HANDLER_STANDARD_H

#include "define.h"
#include "netcode.h"
#include "crc.h"
#include "ack_mgr_standard.h"

STRUCT_DECLARE(UdpSocket);
STRUCT_DECLARE(UdpClient);
STRUCT_DECLARE(ProtocolHandler);

STRUCT_DEFINE(ProtocolHandlerStandard)
{
    AckMgrStandard  ackMgr;
};

void    protocol_handler_standard_init(R(UdpSocket*) sock, R(UdpClient*) client, R(ProtocolHandler*) handler, uint32_t index);
void    protocol_handler_standard_deinit(R(ProtocolHandlerStandard*) handler);

void    protocol_handler_standard_recv(R(ProtocolHandlerStandard*) handler, R(byte*) data, int len);

#endif//EQP_PROTOCOL_HANDLER_STANDARD_H
