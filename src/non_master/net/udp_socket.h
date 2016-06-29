
#ifndef EQP_UDP_SOCKET_H
#define EQP_UDP_SOCKET_H

#include "define.h"
#include "netcode.h"
#include "eqp_array.h"
#include "udp_client.h"
#include "eqp_log.h"
#include "protocol_handler.h"

#define EQP_UDP_SOCKET_BUFFER_SIZE 1024
#define EQP_UDP_SOCKET_LINKDEAD_TIMEOUT_MILLISECONDS 15000

STRUCT_DECLARE(Basic);

STRUCT_DEFINE(UdpSocket)
{
    int     socketFd;
    Basic*  basic;
    Array*  clients;
    
    byte    recvBuffer[EQP_UDP_SOCKET_BUFFER_SIZE];
    byte    compressBuffer[EQP_UDP_SOCKET_BUFFER_SIZE];
};

UdpSocket*  udp_socket_create(Basic* basic);
void        udp_socket_destroy(UdpSocket* sock);

void        udp_socket_open(UdpSocket* sock, uint16_t port);
void        udp_socket_close(UdpSocket* sock);
void        udp_socket_recv(UdpSocket* sock);
void        udp_socket_check_timeouts(UdpSocket* sock);
void        udp_socket_send(UdpSocket* sock);

#define     udp_socket_fd(sock) ((sock)->socketFd)
#define     udp_socket_basic(sock) ((sock)->basic)
#define     udp_socket_get_compress_buffer(sock) ((sock)->compressBuffer)

void        udp_socket_flag_client_as_dead_by_index(UdpSocket* sock, uint32_t index);

#endif//EQP_UDP_SOCKET_H
