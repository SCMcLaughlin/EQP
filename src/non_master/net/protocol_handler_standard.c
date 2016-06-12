
#include "protocol_handler_standard.h"
#include "protocol_handler.h"
#include "udp_socket.h"
#include "udp_client.h"
#include "eqp_basic.h"

void protocol_handler_standard_init(R(UdpSocket*) sock, R(UdpClient*) client, R(ProtocolHandler*) handler, uint32_t index)
{
    ack_mgr_standard_init(sock, client, &handler->standard.ackMgr, index);
    
    //handler->standard.clientObject = client_create_from_new_connection_standard(handler);
}

void protocol_handler_standard_deinit(R(ProtocolHandlerStandard*) handler)
{
    ack_mgr_standard_deinit(&handler->ackMgr);
}

static void protocol_handler_standard_handle_session_request(R(ProtocolHandlerStandard*) handler, R(Aligned*) a)
{
    
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
    
    // Packets that aren't subject to verification and encoding checks
    switch (protocolOpcode)
    {
    case ProtocolOp_SessionRequest:
        //protocol_handler_standard_handle_session_request(handler, a);
        return;
    
    case ProtocolOp_SessionDisconnect:
        //protocol_handler_standard_disconnect(handler);
        return;
    
    default:
        break;
    }
    
    //if (!protocol_handler_standard_validate_and_decompress_packet(handler, a, isFromCombined))
    //    return;
    
    switch (protocolOpcode)
    {
    case ProtocolOp_SessionStatsRequest:
        //protocol_handler_standard_handle_session_stats_request(handler, a);
        break;
    
    case ProtocolOp_Combined:
        //protocol_handler_standard_handle_combined(handler, a);
        break;
    
    case ProtocolOp_Packet:
        //ack_mgr_standard_check_sequence_packet(handler, a);
        break;
    
    case ProtocolOp_Fragment:
        //ack_mgr_standard_check_sequence_fragment(handler, a);
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
    //aligned_write_uint32(&w, sessionId from somewhere);
    
    //protocol_handler_standard_send_packet_immediate(handler, &dis, sizeof(Protocol_SessionDisconnect));
    
    //request destruction from udpsocket
}
