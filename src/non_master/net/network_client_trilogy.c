
#include "network_client_trilogy.h"
#include "udp_socket.h"

void network_client_trilogy_init(R(UdpSocket*) sock, R(UdpClient*) udpClient, R(NetworkClientTrilogy*) client)
{
    network_client_init(sock, udpClient, &client->base);
    
    client->outputPackets       = array_create_type(udp_socket_basic(sock), OutputPacketTrilogy);
    client->pendingAckResponses = array_create_type(udp_socket_basic(sock), uint16_t);
    
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
    
    if (client->pendingAckResponses)
    {
        array_destroy(client->pendingAckResponses);
        client->pendingAckResponses = NULL;
    }
}

void network_client_trilogy_recv_ack_request(R(NetworkClientTrilogy*) client, uint16_t ack)
{
    uint16_t* back = array_back_type(client->pendingAckResponses, uint16_t);
    
    if (!back || ack != *back)
        array_push_back(network_client_trilogy_basic(client), &client->pendingAckResponses, &ack);
}

void network_client_trilogy_schedule_packet(R(NetworkClientTrilogy*) client, R(OutputPacketTrilogy*) packet)
{
    array_push_back(network_client_trilogy_basic(client), &client->outputPackets, packet);
}

void network_client_trilogy_send_queued(R(NetworkClientTrilogy*) client)
{
    Aligned aligned;
    R(Aligned*) a                   = &aligned;
    uint32_t n                      = array_count(client->outputPackets);
    R(OutputPacketTrilogy*) array   = array_data_type(client->outputPackets, OutputPacketTrilogy);
    uint32_t m                      = array_count(client->pendingAckResponses);
    R(uint16_t*) ackResponses       = array_data_type(client->pendingAckResponses, uint16_t);
    uint32_t j                      = 0;
    uint32_t i;
    
    aligned_init(network_client_trilogy_basic(client), a, NULL, 0);
    
    for (i = client->sendFromIndex; i < n; i++)
    {
        R(OutputPacketTrilogy*) wrapper = &array[i];
        uint16_t header                 = wrapper->header;
        R(PacketTrilogy*) packet        = wrapper->packet;
        uint32_t len                    = packet_trilogy_length(packet);
        
        aligned_reinit_cursor(a, packet_trilogy_data_raw(packet), EQP_PACKET_TRILOGY_DATA_OFFSET + len + sizeof(uint32_t), EQP_PACKET_TRILOGY_DATA_OFFSET);
        
        // opcode
        aligned_write_reverse_uint16(a, packet_trilogy_opcode(packet));
        
        // ackCounters
        if (header & PacketTrilogyHasAckRequest)
        {
            aligned_write_reverse_uint8(a, wrapper->ackCounterRequest);
            aligned_write_reverse_uint8(a, wrapper->ackCounterAlwaysOne);
        }
        
        // fragment info
        if (header & PacketTrilogyIsFragment)
        {
            
        }
        
        // ackRequest
        if (header & PacketTrilogyHasAckRequest)
            aligned_write_reverse_uint16(a, toNetworkUint16(wrapper->ackRequest));
        
        // ackResponse
        if (j < m)
        {
            header |= PacketTrilogyHasAckResponse;
            aligned_write_reverse_uint16(a, ackResponses[j++]);
        }
        
        // sequence
        aligned_write_reverse_uint16(a, toNetworkUint16(wrapper->seq));
        
        // header
        aligned_write_reverse_uint16(a, header);
        
        // crc
        {
            R(byte*) data   = packet_trilogy_data_raw(packet) + aligned_position(a);
            uint32_t clen   = aligned_remaining(a) - sizeof(uint32_t);
            uint32_t crc    = crc_calc32_network(data, clen);
            
            aligned_advance(a, clen);
            aligned_write_uint32(a, crc);
        
            network_client_send(&client->base, data, clen + sizeof(uint32_t));
        }
    }
    
    while (j < m)
    {
        //send extra pure-ack packets here
        j++;
    }
    
    client->sendFromIndex = i;
    array_clear(client->pendingAckResponses);
}
