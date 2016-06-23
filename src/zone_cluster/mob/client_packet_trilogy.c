
#include "client_packet_trilogy.h"

void client_recv_packet_trilogy(R(void*) client, uint16_t opcode, R(Aligned*) a)
{
    printf("Opcode: 0x%04x, length: %u\n", opcode, aligned_remaining(a));
}
