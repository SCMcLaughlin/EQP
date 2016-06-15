
#include "tcp_client.h"
#include "eqp_login.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"

#define SERVER_UP 0
#define SERVER_LOCKED -2

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

static void tcp_client_send(R(Login*) login, R(TcpClient*) client, R(const void*) data, int size)
{
    int fd      = client->socketFd;
    int sent    = 0;
    
    for (;;)
    {
        int len = send(fd, ((byte*)data) + sent, size - sent, 0);
        
        if (len == -1)
        {
            int err = errno;
            if (err != EAGAIN && err != EWOULDBLOCK)
                log_format(B(login), LogNetwork, "[tcp_client_keep_alive] send() syscall failed, errno %i", err);
            break;
        }
        
        sent += len;
        
        if (sent == size)
            break;
    }
}

static void tcp_client_keep_alive(R(Login*) login, R(TcpClient*) client)
{
    TcpPacketHeader packet;

    packet.opcode = TcpOp_KeepAlive;
    packet.length = sizeof(TcpPacketHeader);
    
    tcp_client_send(login, client, &packet, sizeof(TcpPacketHeader));
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
    char remoteIp[INET_ADDRSTRLEN];
    
    if (tcp_client_has_login_server(client) || aligned_remaining(a) < sizeof(Tcp_NewLoginServer))
        return;
    
    if (!inet_ntop(AF_INET, &tcp_client_address(client), remoteIp, sizeof(IpAddress)))
    {
        int err = errno;
        log_format(B(login), LogNetwork, "[tcp_client_handle_op_new_login_server] inet_ntop() syscall failed, errno %i\n", err);
        remoteIp[0] = 0;
    }
    
    server.longName         = tcp_client_string_from_fixed_field(B(login), a, sizeof_field(Tcp_NewLoginServer, longName));
    server.shortName        = tcp_client_string_from_fixed_field(B(login), a, sizeof_field(Tcp_NewLoginServer, shortName));
    aligned_advance(a, sizeof_field(Tcp_NewLoginServer, remoteAddress));
    server.localIpAddress   = tcp_client_string_from_fixed_field(B(login), a, sizeof_field(Tcp_NewLoginServer, localAddress));
    
    server.remoteIpAddress  = string_create_from_cstr(B(login), remoteIp, strlen(remoteIp));
    
    server.status           = ServerStatus_Locked;
    server.rank             = ServerRank_Standard;
    server.playerCount      = 0;
    
    client->loginServerIndex = server_list_add(login_server_list(login), &server);
}

static void tcp_client_handle_op_login_server_status(R(Login*) login, R(TcpClient*) client, R(Aligned*) a)
{
    int status;
    int playerCount;
    int zones;
    
    if (!tcp_client_has_login_server(client) || aligned_remaining(a) < sizeof(Tcp_LoginServerStatus))
        return;
    
    status      = aligned_read_int32(a);
    playerCount = aligned_read_int32(a);
    zones       = aligned_read_int32(a);
    
    switch (status)
    {
    case SERVER_UP:
        status = ServerStatus_Up;
        break;
    
    case SERVER_LOCKED:
        status = ServerStatus_Locked;
        break;
    
    default:
        status = ServerStatus_Down;
        break;
    }
    
    server_list_update_by_index(login_server_list(login), tcp_client_login_server_index(client), playerCount, status);
    
    printf("status: %i, playerCount: %i, zones: %i\n", status, playerCount, zones);
    
    tcp_client_keep_alive(login, client);
}

static void tcp_client_send_client_auth(R(Login*) login, R(TcpClient*) client, R(LoginClient*) loginClient)
{
    Tcp_ClientLoginAuth auth;
    Aligned write;
    R(Aligned*) w = &write;
    
    aligned_init(B(login), w, &auth, sizeof(Tcp_ClientLoginAuth));
    
    login_client_generate_session_key(loginClient);
    
    //fixme: loginAdmin, serverAdmin, isLocal need to be determined
    
    // header.opcode
    aligned_write_uint16(w, TcpOp_ClientLoginAuth);
    // header.length
    aligned_write_uint16(w, sizeof(Tcp_ClientLoginAuth));
    // accountId
    aligned_write_uint32(w, login_client_account_id(loginClient));
    // accountName
    aligned_write_snprintf_full_advance(w, sizeof_field(Tcp_ClientLoginAuth, accountName), "%s", string_data(login_client_account_name(loginClient)));
    // sessionKey
    aligned_write_snprintf_full_advance(w, sizeof_field(Tcp_ClientLoginAuth, sessionKey), "%s", login_client_session_key(loginClient));
    // loginAdmin
    aligned_write_uint8(w, 0);
    // serverAdmin
    aligned_write_int16(w, 0);
    // ip
    aligned_write_uint32(w, protocol_handler_ip_address(login_client_handler(loginClient))->sin_addr.s_addr);
    // isLocal
    aligned_write_uint8(w, true);

    tcp_client_send(login, client, &auth, sizeof(Tcp_ClientLoginAuth));
}

static void tcp_client_handle_op_client_login_response(R(Login*) login, R(TcpClient*) client, R(Aligned*) a)
{
    R(LoginClient*) loginClient;
    uint32_t accountId;
    int response;
    
    if (!tcp_client_has_login_server(client) || aligned_remaining(a) < sizeof(Tcp_ClientLoginResponse))
        return;
    
    // accountId
    accountId = aligned_read_uint32(a);
    // serverId
    aligned_advance(a, sizeof(uint32_t));
    // response
    response = aligned_read_int8(a);
    
    loginClient = client_list_get(login_client_list(login), accountId);
    
    if (!loginClient)
        return;
    
    if (response == LOGIN_RESPONSE_ACCEPTED)
        tcp_client_send_client_auth(login, client, loginClient);
    
    login_client_handle_login_response(loginClient, response);
}

void tcp_client_handle_packet(R(Login*) login, R(TcpClient*) client)
{
    Aligned aligned;
    R(Aligned*) a = &aligned;
    uint16_t opcode;
    
    aligned_init(B(login), a, client->recvBuf, client->readLength);
    
    opcode = aligned_read_uint16(a);
    aligned_advance(a, sizeof(uint16_t)); // We already know the packet's data length
    
    printf("TCP packet, opcode: 0x%04x, length: %u\n", opcode, aligned_remaining(a));
    
    switch (opcode)
    {
    case TcpOp_NewLoginServer:
        tcp_client_handle_op_new_login_server(login, client, a);
        break;
    
    case TcpOp_LoginServerStatus:
        tcp_client_handle_op_login_server_status(login, client, a);
        break;
    
    case TcpOp_ClientLoginResponse:
        tcp_client_handle_op_client_login_response(login, client, a);
        break;
    
    default:
        break;
    }
}

void tcp_client_send_client_login_request(R(Login*) login, R(TcpClient*) client, uint32_t accountId)
{
    // This struct is fully aligned
    Tcp_ClientLoginRequest req;
    
    if (!tcp_client_has_login_server(client))
        return;
    
    req.header.opcode   = TcpOp_ClientLoginRequest;
    req.header.length   = sizeof(Tcp_ClientLoginRequest);
    req.accountId       = accountId;
    req.serverId        = 0;
    req.unused[0]       = 0;
    req.unused[1]       = 0;
    
    tcp_client_send(login, client, &req, sizeof(Tcp_ClientLoginRequest));
}

#undef SERVER_UP
#undef SERVER_LOCKED
