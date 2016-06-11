
#include "tcp_client.h"
#include "eqp_login.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"

void tcp_client_init(R(Basic*) basic, R(TcpClient*) client, int fd, R(IpAddress*) addr)
{
    client->socketFd            = fd;
    client->buffered            = 0;
    client->readLength          = sizeof(TcpPacketHeader);
    client->loginServerIndex    = -1;
    client->address             = *addr;
    client->recvBuf             = eqp_alloc_type_bytes(basic, EQP_TCP_CLIENT_BUFFER_SIZE, byte);
}

void tcp_client_deinit(R(TcpClient*) client)
{
    if (client->socketFd != INVALID_SOCKET)
    {
        closesocket(client->socketFd);
        client->socketFd = INVALID_SOCKET;
    }
    
    if (client->recvBuf)
    {
        free(client->recvBuf);
        client->recvBuf = NULL;
    }
}

static String* tcp_client_string_from_fixed_field(R(Basic*) basic, R(Aligned*) a, uint32_t fieldLength)
{
    R(const char*) str  = aligned_current_type(a, const char);
    uint32_t len        = strlen(str);
    R(String*) out;
    
    if (len >= fieldLength)
        len = fieldLength - 1;
    
    out = string_create_from_cstr(basic, str, len);
    
    aligned_advance(a, fieldLength);
    
    printf("field: %s\n", str);
    
    return out;
}

static void tcp_client_handle_op_new_login_server(R(Login*) login, R(TcpClient*) client, R(Aligned*) a)
{
    ServerListing server;
    
    if (tcp_client_has_login_server(client) || aligned_remaining(a) < sizeof(Tcp_NewLoginServer))
        return;
    
    server.longName     = tcp_client_string_from_fixed_field(B(login), a, sizeof_field(Tcp_NewLoginServer, longName));
    server.shortName    = tcp_client_string_from_fixed_field(B(login), a, sizeof_field(Tcp_NewLoginServer, shortName));
    aligned_advance(a, sizeof_field(Tcp_NewLoginServer, remoteAddress));
    server.ipAddress    = tcp_client_string_from_fixed_field(B(login), a, sizeof_field(Tcp_NewLoginServer, localAddress));
    
    server.status       = ServerStatus_Locked;
    server.rank         = ServerRank_Standard;
    server.playerCount  = 0;
    
    client->loginServerIndex = server_list_add(login_server_list(login), &server);
}

static void tcp_client_handle_op_login_server_status(R(Login*) login, R(TcpClient*) client, R(Aligned*) a)
{
    (void)login;
    (void)client;
    
    int status      = aligned_read_int32(a);
    int playerCount = aligned_read_int32(a);
    int zones       = aligned_read_int32(a);
    
    printf("status: %i, playerCount: %i, zones: %i\n", status, playerCount, zones);
}

void tcp_client_handle_packet(R(Login*) login, R(TcpClient*) client)
{
    Aligned aligned;
    R(Aligned*) a = &aligned;
    uint16_t opcode;
    
    aligned_init(B(login), a, client->recvBuf, client->readLength);
    
    opcode = aligned_read_uint16(a);
    aligned_advance(a, sizeof(uint16_t)); // We already implicitly know the packet's data length
    
    printf("TCP packet, opcode: 0x%04x, length: %u\n", opcode, aligned_remaining(a));
    
    switch (opcode)
    {
    case TcpOp_NewLoginServer:
        tcp_client_handle_op_new_login_server(login, client, a);
        break;
    
    case TcpOp_LoginServerStatus:
        tcp_client_handle_op_login_server_status(login, client, a);
        break;
    
    default:
        break;
    }
}
