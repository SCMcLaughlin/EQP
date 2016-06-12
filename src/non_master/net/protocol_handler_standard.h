
#ifndef EQP_PROTOCOL_HANDLER_STANDARD_H
#define EQP_PROTOCOL_HANDLER_STANDARD_H

#include "define.h"
#include "netcode.h"
#include "crc.h"
#include "aligned.h"
#include "ack_mgr_standard.h"
#include "network_interface.h"
#include "protocol_opcodes.h"
#include "protocol_structs.h"

STRUCT_DECLARE(UdpSocket);
STRUCT_DECLARE(UdpClient);
STRUCT_DECLARE(ProtocolHandler);

STRUCT_DEFINE(ProtocolHandlerStandard)
{
    AckMgrStandard  ackMgr;
    void*           clientObject;
};

void    protocol_handler_standard_init(R(UdpSocket*) sock, R(UdpClient*) client, R(ProtocolHandler*) handler, uint32_t index);
void    protocol_handler_standard_deinit(R(ProtocolHandlerStandard*) handler);

void    protocol_handler_standard_recv(R(ProtocolHandlerStandard*) handler, R(byte*) data, int len);
void    protocol_handler_standard_handle_recv(R(ProtocolHandlerStandard*) handler, R(Aligned*) a, int isFromCombined);
void    protocol_handler_standard_disconnect(R(ProtocolHandlerStandard*) handler);

#define protocol_handler_standard_basic(handler) ack_mgr_standard_basic(&(handler)->ackMgr)

#define protocol_handler_standard_increment_packets_received(handler) ack_mgr_standard_increment_packets_received(&(handler)->ackMgr)

#endif//EQP_PROTOCOL_HANDLER_STANDARD_H
