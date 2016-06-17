
#include "network_client_trilogy.h"
#include "udp_socket.h"

void network_client_trilogy_init(R(UdpSocket*) sock, R(UdpClient*) udpClient, R(NetworkClientTrilogy*) client, uint32_t index)
{
    network_client_init(sock, udpClient, &client->base, index);
    
    client->outputPackets           = array_create_type(udp_socket_basic(sock), OutputPacketTrilogy);
    client->nextAckResponse         = 0;
    client->nextAckRequestExpected  = 0;
    client->nextSeqToSend           = 0;
    client->lastAckReceived         = 0;
    client->sendFromIndex           = 0;
}

void network_client_trilogy_deinit(R(NetworkClientTrilogy*) client)
{
    network_client_deinit(&client->base);
    
    if (client->outputPackets)
    {
        R(OutputPacketTrilogy*) array   = array_data_type(client->outputPackets, OutputPacketTrilogy);
        uint32_t n                      = array_count(client->outputPackets);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            R(PacketTrilogy*) packet = array[i].packet;
            
            if (packet)
                packet_trilogy_drop(packet);
        }
        
        array_destroy(client->outputPackets);
        client->outputPackets = NULL;
    }
}

void network_client_trilogy_recv_ack_response(R(NetworkClientTrilogy*) client, uint16_t ack)
{
    R(OutputPacketTrilogy*) array   = array_data_type(client->outputPackets, OutputPacketTrilogy);
    uint32_t n                      = client->sendFromIndex;
    uint32_t i;
    
    ack                     = toHostUint16(ack);
    client->lastAckReceived = ack;
    
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

void network_client_trilogy_recv_ack_request(R(NetworkClientTrilogy*) client, uint16_t ack, int isFirstPacket)
{
    if (isFirstPacket || (ack == (client->nextAckRequestExpected + 1)))
    {
        client->nextAckResponse         = ack;
        client->nextAckRequestExpected  = ack;
    }
}

void network_client_trilogy_schedule_packet(R(NetworkClientTrilogy*) client, R(OutputPacketTrilogy*) packet)
{
    packet_trilogy_grab(packet->packet);
    array_push_back(network_client_trilogy_basic(client), &client->outputPackets, packet);
}

void network_client_trilogy_send_pure_ack(R(NetworkClientTrilogy*) client, uint16_t ackResponse)
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
    uint16_t ackReceived            = client->lastAckReceived;
    uint32_t sendFromIndex          = client->sendFromIndex;
    uint64_t time                   = clock_milliseconds();
    uint32_t i;
    
    aligned_set_basic(a, network_client_trilogy_basic(client));
    
    for (i = 0; i < n; i++)
    {
        R(OutputPacketTrilogy*) wrapper = &array[i];
        R(PacketTrilogy*) packet        = wrapper->packet;
        uint32_t dataLength             = packet_trilogy_length(packet);
        uint16_t opcode                 = packet_trilogy_opcode(packet);
        uint16_t fragCount              = wrapper->fragCount;
        uint16_t fragIndex              = 0;
        
        aligned_reinit(a, packet_trilogy_data_raw(packet), packet_trilogy_length_raw(packet));
        
        if (i < sendFromIndex)
        {
            // Should we re-send this un-acked packet?
            if ((time - wrapper->ackTimestamp) < EQP_TRILOGY_RESEND_MILLISECONDS)
                continue;
            
            // Figure out if we were to re-send from if we were last acked in the middle of a fragmented packet
            if (fragCount > 1)
            {
                uint16_t ack = wrapper->ackRequest;
                
                while (ack_compare(ackReceived, ack) != AckPast && fragIndex < fragCount)
                {
                    aligned_advance(a, EQP_PACKET_TRILOGY_DATA_OFFSET + EQP_PACKET_TRILOGY_DATA_SPACE - sizeof(uint16_t) + sizeof(uint32_t));
                    
                    if (fragIndex > 0)
                        aligned_advance(a, sizeof(uint16_t));
                    
                    fragIndex++;
                    ack++;
                }
            }
        }
        
        for (; fragIndex < fragCount; fragIndex++)
        {
            uint32_t space = (fragIndex == 0) ? (EQP_PACKET_TRILOGY_DATA_SPACE - sizeof(uint16_t)) : EQP_PACKET_TRILOGY_DATA_SPACE;
            
            aligned_advance(a, EQP_PACKET_TRILOGY_DATA_OFFSET);
            
            network_client_trilogy_send_fragment(&client->base, wrapper, a,
                (dataLength > space) ? space : dataLength, opcode, ackResponse, fragIndex);
            
            wrapper->ackTimestamp = clock_milliseconds();
            
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
