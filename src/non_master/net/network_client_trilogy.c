
#include "network_client_trilogy.h"
#include "udp_socket.h"

void network_client_trilogy_init(R(UdpSocket*) sock, R(UdpClient*) udpClient, R(NetworkClientTrilogy*) client)
{
    network_client_init(sock, udpClient, &client->base);
    
    client->outputPackets   = array_create_type(udp_socket_basic(sock), OutputPacketTrilogy);
    client->nextAckResponse = 0;
    client->nextSeqToSend   = 0;
    client->sendFromIndex   = 0;
    client->sendFromFrag    = 0;
}

void network_client_trilogy_deinit(R(NetworkClientTrilogy*) client)
{
    network_client_deinit(&client->base);
    
    if (client->outputPackets)
    {
        array_destroy(client->outputPackets);
        client->outputPackets = NULL;
    }
}

void network_client_trilogy_recv_ack_response(R(NetworkClientTrilogy*) client, uint16_t ack)
{
    R(OutputPacketTrilogy*) array   = array_data_type(client->outputPackets, OutputPacketTrilogy);
    uint32_t n                      = client->sendFromIndex;
    uint32_t i;
    
    ack = toHostUint16(ack);
    
    for (i = 0; i < n; i++)
    {
        R(OutputPacketTrilogy*) wrapper = &array[i];
        uint16_t ackRequest             = wrapper->ackRequest + (wrapper->fragCount - 1);
        
        if (ackRequest > ack)
            break;
        
        if (wrapper->packet)
        {
            packet_trilogy_drop(wrapper->packet);
            wrapper->packet = NULL;
        }
    }
    
    if (i > 0)
    {
        array_shift_left(client->outputPackets, i);
        client->sendFromIndex = 0;
    }
}

void network_client_trilogy_recv_ack_request(R(NetworkClientTrilogy*) client, uint16_t ack)
{
    ack = toHostUint16(ack);
    
    if (ack > client->nextAckResponse)
        client->nextAckResponse = ack;
}

void network_client_trilogy_schedule_packet(R(NetworkClientTrilogy*) client, R(OutputPacketTrilogy*) packet)
{
    packet_trilogy_grab(packet->packet);
    array_push_back(network_client_trilogy_basic(client), &client->outputPackets, packet);
}

static void network_client_trilogy_send_pure_ack(R(NetworkClientTrilogy*) client, uint16_t ackResponse)
{
    byte buf[10];
    Aligned write;
    Aligned* w      = &write;
    uint16_t header = PacketTrilogyHasAckResponse;
    
    aligned_init(network_client_trilogy_basic(client), w, buf, sizeof(buf));
    
    aligned_write_uint16(w, header);
    aligned_write_uint16(w, toNetworkUint16(client->nextSeqToSend++));
    aligned_write_uint16(w, ackResponse);
    aligned_write_uint32(w, crc_calc32_network(buf, sizeof(buf) - sizeof(uint32_t)));
    
    network_client_send(&client->base, buf, sizeof(buf));
}

static void network_client_trilogy_send_fragment(R(NetworkClient*) client, R(OutputPacketTrilogy*) wrapper, R(Aligned*) a,
    uint32_t dataLength, uint16_t opcode, uint16_t ackResponse, uint32_t fragIndex)
{
    uint32_t orig   = aligned_position(a);
    uint16_t header = wrapper->header;
    R(byte*) data;
    
    // opcode
    if (fragIndex == 0 && opcode)
        aligned_write_reverse_uint16(a, opcode);
    
    // ackCounters
    if (header & PacketTrilogyHasAckRequest)
    {
        aligned_write_reverse_uint8(a, wrapper->ackCounterRequest + fragIndex);
        aligned_write_reverse_uint8(a, wrapper->ackCounterAlwaysOne);
    }
    
    // fragment info
    if (header & PacketTrilogyIsFragment)
    {
        aligned_write_reverse_uint16(a, toNetworkUint16(wrapper->fragCount));
        aligned_write_reverse_uint16(a, toNetworkUint16(fragIndex));
        aligned_write_reverse_uint16(a, toNetworkUint16(wrapper->fragGroup));
    }
    
    // ackRequest
    if (header & PacketTrilogyHasAckRequest)
        aligned_write_reverse_uint16(a, toNetworkUint16(wrapper->ackRequest + fragIndex));
    
    // ackResponse
    if (ackResponse)
    {
        header |= PacketTrilogyHasAckResponse;
        aligned_write_reverse_uint16(a, ackResponse);
    }
    
    // sequence
    aligned_write_reverse_uint16(a, toNetworkUint16(wrapper->seq + fragIndex));
    
    // header
    aligned_write_reverse_uint16(a, header);
    
    // crc
    data = aligned_current(a);
    dataLength += orig - aligned_position(a);
    
    aligned_advance(a, dataLength);
    aligned_write_uint32(a, crc_calc32_network(data, dataLength));
    
    network_client_send(client, data, dataLength + sizeof(uint32_t));
}

void network_client_trilogy_send_queued(R(NetworkClientTrilogy*) client)
{
    Aligned aligned;
    R(Aligned*) a                   = &aligned;
    uint32_t n                      = array_count(client->outputPackets);
    R(OutputPacketTrilogy*) array   = array_data_type(client->outputPackets, OutputPacketTrilogy);
    uint16_t ackResponse            = toNetworkUint16(client->nextAckResponse);
    uint32_t i;
    
    aligned_set_basic(a, network_client_trilogy_basic(client));
    
    for (i = client->sendFromIndex; i < n; i++)
    {
        R(OutputPacketTrilogy*) wrapper = &array[i];
        R(PacketTrilogy*) packet        = wrapper->packet;
        uint32_t dataLength             = packet_trilogy_length(packet);
        uint16_t opcode                 = packet_trilogy_opcode(packet);
        uint16_t fragCount              = wrapper->fragCount;
        uint16_t fragIndex;
        
        aligned_reinit(a, packet_trilogy_data_raw(packet), packet_trilogy_length_raw(packet));
        
        for (fragIndex = 0; fragIndex < fragCount; fragIndex++)
        {
            uint32_t space = (fragIndex == 0) ? (EQP_PACKET_TRILOGY_DATA_SPACE - sizeof(uint16_t)) : EQP_PACKET_TRILOGY_DATA_SPACE;
            
            aligned_advance(a, EQP_PACKET_TRILOGY_DATA_OFFSET);
            
            network_client_trilogy_send_fragment(&client->base, wrapper, a,
                (dataLength > space) ? space : dataLength, opcode, ackResponse, fragIndex);
            
            ackResponse = 0;
            dataLength -= space;
        }
    }
    
    if (ackResponse)
        network_client_trilogy_send_pure_ack(client, ackResponse);
    
    client->nextAckResponse = 0;
    client->sendFromIndex   = i;
}

uint16_t network_client_trilogy_get_next_seq_to_send_and_increment(R(NetworkClientTrilogy*) client, uint16_t by)
{
    uint16_t seq = client->nextSeqToSend;
    client->nextSeqToSend += by;
    return seq;
}
