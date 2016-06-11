
#ifndef EQP_UDP_SOCKET_H
#define EQP_UDP_SOCKET_H

#include "define.h"
#include "netcode.h"
#include "eqp_array.h"
#include "udp_client.h"
#include "eqp_log.h"
#include "protocol_handler.h"

#define EQP_UDP_SOCKET_BUFFER_SIZE 1024
#define EQP_UDP_SOCKET_LINKDEAD_TIMEOUT_MILLISECONDS 30000

STRUCT_DECLARE(Basic);

STRUCT_DEFINE(UdpSocket)
{
    int     socketFd;
    byte    recvBuffer[EQP_UDP_SOCKET_BUFFER_SIZE];
    
    Basic*  basic;
    Array*  clients;
    
    byte    compressBuffer[EQP_UDP_SOCKET_BUFFER_SIZE];
    byte    decompressBuffer[EQP_UDP_SOCKET_BUFFER_SIZE];
};

UdpSocket*  udp_socket_create(R(Basic*) basic);
void        udp_socket_destroy(R(UdpSocket*) sock);

void        udp_socket_open(R(UdpSocket*) sock, uint16_t port);
void        udp_socket_close(R(UdpSocket*) sock);
void        udp_socket_recv(R(UdpSocket*) sock);
void        udp_socket_check_timeouts(R(UdpSocket*) sock);
void        udp_socket_send(R(UdpSocket*) sock);

#define     udp_socket_fd(sock) ((sock)->socketFd)
#define     udp_socket_basic(sock) ((sock)->basic)

void        udp_socket_flag_client_as_dead_by_index(R(UdpSocket*) sock, uint32_t index);

#endif//EQP_UDP_SOCKET_H
