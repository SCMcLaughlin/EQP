
#include "protocol_handler_standard.h"
#include "protocol_handler.h"
#include "udp_socket.h"
#include "udp_client.h"
#include "eqp_basic.h"

void protocol_handler_standard_init(R(UdpSocket*) sock, R(UdpClient*) client, R(ProtocolHandler*) handler, uint32_t index)
{
    (void)sock;
    (void)client;
    (void)handler;
    (void)index;
}

void protocol_handler_standard_deinit(R(ProtocolHandlerStandard*) handler)
{
    (void)handler;
}

void protocol_handler_standard_recv(R(ProtocolHandlerStandard*) handler, R(byte*) data, int len)
{
    (void)handler;
    (void)data;
    (void)len;
}
