
#include "ack_mgr_trilogy.h"

void ack_mgr_trilogy_init(R(UdpSocket*) sock, R(UdpClient*) client, R(AckMgrTrilogy*) ackMgr, uint32_t index)
{
    network_client_trilogy_init(sock, client, &ackMgr->client, index);
    
    ackMgr->nextAckToRequest    = 0;
    ackMgr->nextSeqToReceive    = 0;
    ackMgr->nextFragGroup       = 0;
    
    ackMgr->ackCounterAlwaysOne = 1;
    ackMgr->ackCounterRequest   = 0;
}

void ack_mgr_trilogy_deinit(R(AckMgrTrilogy*) ackMgr)
{
    network_client_trilogy_deinit(&ackMgr->client);
}

void ack_mgr_trilogy_schedule_packet(R(AckMgrTrilogy*) ackMgr, R(PacketTrilogy*) packet, int noAckRequest)
{
    OutputPacketTrilogy wrapper;
    uint16_t header     = 0;
    uint16_t fragCount  = packet_trilogy_frag_count(packet);
    noAckRequest        = (noAckRequest && fragCount == 0);
    
    fragCount++;
    
    memset(&wrapper, 0, sizeof(OutputPacketTrilogy));
    
    wrapper.fragCount   = fragCount;
    wrapper.packet      = packet;
    wrapper.seq         = network_client_trilogy_get_next_seq_to_send_and_increment(&ackMgr->client, fragCount);
    
    if (!noAckRequest)
    {
        if (ackMgr->nextAckToRequest == 0)
        {
            header |= PacketTrilogyIsFirstPacket;
            ackMgr->nextAckToRequest = random_uint16() | 1; // Make sure the final value can't be zero
        }
        
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
