
#ifndef EQP_PROTOCOL_HANDLER_STANDARD_H
#define EQP_PROTOCOL_HANDLER_STANDARD_H

#include "define.h"
#include "netcode.h"
#include "crc.h"
#include "aligned.h"
#include "random.h"
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
    uint32_t        crcKey;
    uint32_t        sessionId;
    uint64_t        startTimeMilliseconds;
    uint8_t         protocolValidation;
    uint8_t         protocolFormat;
};

void    protocol_handler_standard_init(UdpSocket* sock, UdpClient* client, ProtocolHandler* handler, uint32_t index);
void    protocol_handler_standard_deinit(ProtocolHandlerStandard* handler);

void    protocol_handler_standard_recv(ProtocolHandlerStandard* handler, byte* data, int len);
void    protocol_handler_standard_handle_recv(ProtocolHandlerStandard* handler, Aligned* a, int isFromCombined);
void    protocol_handler_standard_disconnect(ProtocolHandlerStandard* handler);

#define protocol_handler_standard_basic(handler) ack_mgr_standard_basic(&(handler)->ackMgr)
#define protocol_handler_standard_client_object(handler) ((handler)->clientObject)

#define protocol_handler_standard_update_index(handler, i) ack_mgr_standard_update_index(&(handler)->ackMgr, i)
#define protocol_handler_standard_flag_connection_as_dead(handler) ack_mgr_standard_flag_connection_as_dead(&(handler)->ackMgr)
#define protocol_handler_standard_increment_packets_received(handler) ack_mgr_standard_increment_packets_received(&(handler)->ackMgr)

#define protocol_handler_standard_send_immediate(handler, data, len) ack_mgr_standard_send_immediate(&(handler)->ackMgr, (data), (len))

#endif//EQP_PROTOCOL_HANDLER_STANDARD_H
