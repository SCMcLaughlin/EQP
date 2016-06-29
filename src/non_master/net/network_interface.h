
#ifndef EQP_NETWORK_INTERFACE_H
#define EQP_NETWORK_INTERFACE_H

#include "define.h"
#include "aligned.h"

/*
    This header provides a special set of forward function declarations.
    No implementation is provided for these within the shared source files;
    instead, each executable that does EQ networking (login, char-select, 
    zone-cluster) will provide their own, specific implementations.
    
    This allows each to have different behavior, while also avoiding the
    need to use a function pointer, or otherwise splitting the build
    process in a more awkward way. Each executable will fill in their
    own implementation at link time.
*/

STRUCT_DECLARE(ProtocolHandler);

void*   client_create_from_new_connection_standard(ProtocolHandler* handler);
void*   client_create_from_new_connection_trilogy(ProtocolHandler* handler);

void    client_recv_packet_standard(void* client, uint16_t opcode, Aligned* a);
void    client_recv_packet_trilogy(void* client, uint16_t opcode, Aligned* a);

void    client_on_disconnect(void* client, int isLinkdead);

#endif//EQP_NETWORK_INTERFACE_H
