
#include "ack_mgr_standard.h"

void ack_mgr_standard_init(UdpSocket* sock, UdpClient* client, AckMgrStandard* ackMgr, uint32_t index)
{
    network_client_standard_init(sock, client, &ackMgr->client, index);
}

void ack_mgr_standard_deinit(AckMgrStandard* ackMgr)
{
    network_client_standard_deinit(&ackMgr->client);
}
