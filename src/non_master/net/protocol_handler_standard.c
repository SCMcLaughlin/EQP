
#include "protocol_handler_standard.h"
#include "protocol_handler.h"
#include "udp_socket.h"
#include "udp_client.h"
#include "eqp_basic.h"

#define MAX_LENGTH 512

void protocol_handler_standard_init(R(UdpSocket*) sock, R(UdpClient*) client, R(ProtocolHandler*) handler, uint32_t index)
{
    R(ProtocolHandlerStandard*) h = &handler->standard;
    
    ack_mgr_standard_init(sock, client, &h->ackMgr, index);
    
    //h->clientObject             = client_create_from_new_connection_standard(handler);
    h->crcKey                   = 0;
    h->sessionId                = 0;
    h->startTimeMilliseconds    = 0;
    h->protocolValidation       = ProtocolValidation_None; //fixme: these should be configurable somewhere
    h->protocolFormat           = ProtocolFormat_None;
}

void protocol_handler_standard_deinit(R(ProtocolHandlerStandard*) handler)
{
    ack_mgr_standard_deinit(&handler->ackMgr);
}

static void protocol_handler_standard_handle_session_request(R(ProtocolHandlerStandard*) handler, R(Aligned*) a)
{
    Protocol_SessionResponse response;
    Aligned write;
    R(Aligned*) w = &write;
    
    if (aligned_remaining(a) < sizeof(Protocol_SessionRequest))
        return;
    
    aligned_advance(a, offsetof(Protocol_SessionRequest, sessionId));
    
    handler->crcKey                 = random_uint32() | 1; // Make sure result is non-zero
    handler->sessionId              = aligned_read_uint32(a);
    handler->startTimeMilliseconds  = clock_milliseconds();
    
    aligned_init(protocol_handler_standard_basic(handler), w, &response, sizeof(Protocol_SessionResponse));
    
    // opcode
    aligned_write_uint16(w, toNetworkUint16(ProtocolOp_SessionResponse));
    // sessionId
    aligned_write_uint32(w, handler->sessionId);
    // crcKey
    aligned_write_uint32(w, toNetworkUint32(handler->crcKey));
    // validation
    aligned_write_uint8(w, handler->protocolValidation);
    // format
    aligned_write_uint8(w, handler->protocolFormat);
    // unknownA
    aligned_write_uint8(w, 0);
    // maxLength
    aligned_write_uint32(w, toNetworkUint32(MAX_LENGTH));
    // unknownB
    aligned_write_uint32(w, 0);
    
    protocol_handler_standard_send_immediate(handler, &response, sizeof(Protocol_SessionResponse));
}

static void protocol_handler_standard_handle_combined(R(ProtocolHandlerStandard*) handler, R(Aligned*) a)
{
    Aligned r;
    
    aligned_set_basic(&r, protocol_handler_standard_basic(handler));
    
    while (aligned_remaining(a))
    {
        uint32_t size = aligned_read_uint8(a);
        
        if (size > aligned_remaining(a))
            break;
        
        aligned_reinit(&r, aligned_current(a), size);
        protocol_handler_standard_handle_recv(handler, &r, true);
        aligned_advance(a, size);
    }
}

static int protocol_handler_standard_validate_crc(R(Aligned*) a, uint32_t crcKey)
{
    Aligned r;
    uint32_t check;
    uint32_t len;
    
    if (crcKey == 0)
        goto ret_true;
    
    aligned_init_copy(&r, a);
    len = aligned_remaining(&r) - sizeof(uint16_t);
    aligned_advance(&r, len);
    
    check = toHostUint16(aligned_read_uint16(&r));
    
    if (check == 0)
        goto ret_true;
    
    return (check == crc_calc16(aligned_current(a), len, crcKey));
    
ret_true:
    return true;
}

static int protocol_handler_standard_decompress_packet(R(ProtocolHandlerStandard*) handler, R(Aligned*) a)
{
    (void)handler;
    (void)a;
    return true;
}

static int protocol_handler_standard_validate_and_decompress_packet(R(ProtocolHandlerStandard*) handler, R(Aligned*) a, int isFromCombined)
{
    if (!isFromCombined && handler->protocolValidation == ProtocolValidation_Crc)
    {
        if (!protocol_handler_standard_validate_crc(a, handler->crcKey))
            return false;
        // Don't count the CRC in the length hereafter
        aligned_reduce_size(a, sizeof(uint16_t));
    }
    
    if (handler->protocolFormat == ProtocolFormat_Compressed)
    {
        uint8_t flag = aligned_peek_uint8(a);
        
        if (flag == 0x5a) // Compressed
        {
            if (!protocol_handler_standard_decompress_packet(handler, a))
                return false;
        }
        else if (flag == 0xa5) // Explicitly not compressed
        {
            aligned_advance(a, sizeof(uint8_t));
        }
    }
    
    return true;
}

void protocol_handler_standard_handle_recv(R(ProtocolHandlerStandard*) handler, R(Aligned*) a, int isFromCombined)
{
    uint16_t protocolOpcode = toHostUint16(aligned_read_uint16(a));
    
    if (protocolOpcode > 0xff)
    {
        // Raw, unordered packet, no protocol
        //queue here
        return;
    }
    
    // Opcodes that aren't subject to verification and encoding checks
    switch (protocolOpcode)
    {
    case ProtocolOp_SessionRequest:
        protocol_handler_standard_handle_session_request(handler, a);
        return;
    
    case ProtocolOp_SessionDisconnect:
        protocol_handler_standard_disconnect(handler);
        return;
    
    default:
        break;
    }
    
    if (!protocol_handler_standard_validate_and_decompress_packet(handler, a, isFromCombined))
        return;
    
    switch (protocolOpcode)
    {
    case ProtocolOp_SessionStatsRequest:
        //protocol_handler_standard_handle_session_stats_request(handler, a);
        break;
    
    case ProtocolOp_Combined:
        protocol_handler_standard_handle_combined(handler, a);
        break;
    
    case ProtocolOp_Packet:
        //ack_mgr_standard_check_sequence_packet(handler, a);
        break;
    
    case ProtocolOp_Fragment:
        //ack_mgr_standard_check_sequence_fragment(handler, a);
        break;
    
    case ProtocolOp_OutOfOrder:
        break;
    
    case ProtocolOp_Ack:
        break;
    
    case ProtocolOp_AppCombined:
        break;
    
    default:
        break;
    }
}

void protocol_handler_standard_recv(R(ProtocolHandlerStandard*) handler, R(byte*) data, int len)
{
    Aligned aligned;
    R(Aligned*) a = &aligned;
    
    protocol_handler_standard_increment_packets_received(handler);
    aligned_init(protocol_handler_standard_basic(handler), a, data, len);
    protocol_handler_standard_handle_recv(handler, a, false);
}

void protocol_handler_standard_disconnect(R(ProtocolHandlerStandard*) handler)
{
    Protocol_SessionDisconnect dis;
    Aligned w;
    
    aligned_init(protocol_handler_standard_basic(handler), &w, &dis, sizeof(Protocol_SessionDisconnect));
    
    aligned_write_uint16(&w, toNetworkUint16(ProtocolOp_SessionDisconnect));
    aligned_write_uint32(&w, handler->sessionId); // This is stored in network byte order
    
    protocol_handler_standard_send_immediate(handler, &dis, sizeof(Protocol_SessionDisconnect));
    
    // Request destruction
    protocol_handler_standard_flag_connection_as_dead(handler);
    client_on_disconnect(handler->clientObject, false);
}

#undef MAX_LENGTH
