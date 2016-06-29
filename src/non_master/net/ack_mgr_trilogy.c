
#include "ack_mgr_trilogy.h"
#include "udp_socket.h"

void ack_mgr_trilogy_init(UdpSocket* sock, UdpClient* client, AckMgrTrilogy* ackMgr, uint32_t index)
{
    network_client_trilogy_init(sock, client, &ackMgr->client, index);
    
    ackMgr->nextAckToRequest    = 0;
    ackMgr->nextFragGroup       = 0;
    
    ackMgr->ackCounterAlwaysOne = 1;
    ackMgr->ackCounterRequest   = 0;
    
    ackMgr->inputPackets        = array_create_type(udp_socket_basic(sock), InputPacketTrilogy);
}

void ack_mgr_trilogy_deinit(AckMgrTrilogy* ackMgr)
{
    network_client_trilogy_deinit(&ackMgr->client);
    
    if (ackMgr->inputPackets)
    {
        InputPacketTrilogy* array   = array_data_type(ackMgr->inputPackets, InputPacketTrilogy);
        uint32_t n                  = array_count(ackMgr->inputPackets);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            InputPacketTrilogy* packet = &array[i];
            
            if (packet->data)
            {
                free(packet->data);
                packet->data = NULL;
            }
        }
        
        array_destroy(ackMgr->inputPackets);
        ackMgr->inputPackets = NULL;
    }
}

static uint16_t ack_mgr_trilogy_complete_input_packet(Basic* basic, void* clientObject, InputPacketTrilogy* packet)
{
    Aligned w;
    aligned_init(basic, &w, packet->data, packet->length);
    client_recv_packet_trilogy(clientObject, packet->opcode, &w);
    if (packet->data)
        free(packet->data);
    return packet->ackRequest + 1;
}

static uint16_t ack_mgr_trilogy_complete_input_fragments(Basic* basic, void* clientObject, InputPacketTrilogy* array, uint32_t length, uint32_t i, uint32_t n)
{
    Aligned write;
    Aligned* w = &write;
    InputPacketTrilogy* packet;
    byte* data      = eqp_alloc_type_bytes(basic, length, byte);
    uint16_t opcode = array[i].opcode;
    
    aligned_init(basic, w, data, length);
    
    for (; i <= n; i++)
    {
        packet = &array[i];
        
        aligned_write_buffer(w, packet->data, packet->length);
        
        free(packet->data);
    }
    
    aligned_reset(w);
    client_recv_packet_trilogy(clientObject, opcode, w);
    free(data);
                
    return packet->ackRequest + 1;
}

void ack_mgr_trilogy_recv_packet(AckMgrTrilogy* ackMgr, Aligned* a, void* clientObject, uint16_t opcode, uint16_t ackRequest, uint16_t fragCount)
{
    // Working from the assumption that the client doesn't do anything weird like 
    // interleaving different fragmented packets within sequential ackRequest sequences...
    Basic* basic;
    uint16_t nextAck = ack_mgr_trilogy_next_ack_request_expected(ackMgr);
    uint32_t n;
    uint32_t diff;
    uint32_t i;
    uint32_t length;
    uint32_t frags;
    InputPacketTrilogy input;
    InputPacketTrilogy* ptr;
    
    if (ackRequest == nextAck && fragCount == 0)
    {
        network_client_trilogy_set_next_ack_request_expected(&ackMgr->client, nextAck + 1);
        client_recv_packet_trilogy(clientObject, opcode, a);
        return;
    }
    
    if (ack_compare(ackRequest, nextAck) == AckPast)
        return;
    
    basic   = ack_mgr_trilogy_basic(ackMgr);
    n       = array_count(ackMgr->inputPackets);
    
    memset(&input, 0, sizeof(InputPacketTrilogy));
    
    // Protocol doesn't allow acks to roll over to 0, so shouldn't need to check for ackRequest being lower (fixme: confirm this)
    diff = ackRequest - nextAck;
    
    // Do we need to expand the array?
    while (diff >= n)
    {
        array_push_back(basic, &ackMgr->inputPackets, &input);
        n++;
    }
    
    ptr = array_get_type(ackMgr->inputPackets, diff, InputPacketTrilogy);
    
    // If we already had a packet at this position, abort
    // (The client likes to spam the same packet over and over in some situations, especially during login)
    if (ptr->ackRequest != 0)
        return;
    
    ptr->ackRequest = ackRequest;
    ptr->fragCount  = fragCount;
    ptr->opcode     = opcode;
    ptr->length     = aligned_remaining(a);
    
    if (ptr->length > 0)
    {
        ptr->data = eqp_alloc_type_bytes(basic, ptr->length, byte);
        memcpy(ptr->data, aligned_current(a), ptr->length);
    }
    else
    {
        ptr->data = NULL;
    }
    
    // Check if we have completed any fragmented packets and such
    ptr     = array_data_type(ackMgr->inputPackets, InputPacketTrilogy);
    diff    = 0;
    frags   = 0;
    length  = 0;
    i       = 0;
    
    while (i < n)
    {
        InputPacketTrilogy* packet = &ptr[i];
        
        if (packet->ackRequest == 0)
            break;
        
        fragCount = packet->fragCount;
        
        if (fragCount == 0)
        {
            nextAck = ack_mgr_trilogy_complete_input_packet(basic, clientObject, packet);
            diff++;
        }
        else
        {
            if (fragCount > n)
                break;
            
            length += packet->length;
            frags++;
            
            if (frags == fragCount)
            {
                diff += frags;
                nextAck = ack_mgr_trilogy_complete_input_fragments(basic, clientObject, ptr, length, i - (frags - 1), i);
                length  = 0;
                frags   = 0;
            }
        }
        
        i++;
    }
    
    if (diff > 0)
    {
        array_shift_left(ackMgr->inputPackets, diff);
        network_client_trilogy_set_next_ack_request_expected(&ackMgr->client, nextAck);
    }
}

void ack_mgr_trilogy_schedule_packet(AckMgrTrilogy* ackMgr, PacketTrilogy* packet, int noAckRequest)
{
    OutputPacketTrilogy wrapper;
    uint16_t header     = 0;
    uint16_t fragCount  = packet_trilogy_frag_count(packet);
    noAckRequest        = (noAckRequest && fragCount == 0);
    
    fragCount++;
    
    memset(&wrapper, 0, sizeof(OutputPacketTrilogy));
    
    wrapper.fragCount   = fragCount;
    wrapper.packet      = packet;
    
    if (!noAckRequest)
    {
        if (ackMgr->nextAckToRequest == 0)
            ackMgr->nextAckToRequest = 0x0001; //fixme: do we even need to bother with this? can we just start from 0?
        
        wrapper.ackRequest = ackMgr->nextAckToRequest;
        ackMgr->nextAckToRequest += fragCount;
        
        wrapper.ackCounterAlwaysOne = ackMgr->ackCounterAlwaysOne;
        wrapper.ackCounterRequest   = ackMgr->ackCounterRequest++;
        
        header |= PacketTrilogyHasAckRequest | PacketTrilogyHasAckCounter;
    }
    
    if (fragCount > 1)
    {
        header |= PacketTrilogyIsFragment;
        wrapper.fragGroup = ackMgr->nextFragGroup++;
    }
    
    wrapper.header = header;
    
    network_client_trilogy_schedule_packet(&ackMgr->client, &wrapper);
}
