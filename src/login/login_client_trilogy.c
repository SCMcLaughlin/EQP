
#include "login_client_trilogy.h"
#include "eqp_login.h"

#define ERR_CREDENTIALS "Error: Invalid username/password"
#define SERVER_LIST_COUNT_MAX 255
#define SERVER_DOWN -1
#define SERVER_LOCKED -2

void* client_create_from_new_connection_trilogy(ProtocolHandler* handler)
{
    return login_client_create(handler, ExpansionId_Trilogy, LoginClientTrilogy_BrandNew);
}

static void login_trilogy_schedule_packet(ProtocolHandler* handler, PacketTrilogy* packet)
{
    packet_trilogy_fragmentize(packet);
    protocol_handler_trilogy_schedule_packet(&handler->trilogy, packet);
}

static void login_trilogy_err_msg(ProtocolHandler* handler, const char* msg, uint32_t len)
{
    Basic* basic            = protocol_handler_basic(handler);
    PacketTrilogy* packet   = packet_trilogy_create(basic, TrilogyOp_Error, ++len);
    Aligned w;
    
    aligned_init(basic, &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    aligned_write_buffer(&w, msg, len);
    
    login_trilogy_schedule_packet(handler, packet);
}

#define login_trilogy_err_literal(handler, msg) login_trilogy_err_msg((handler), (msg), sizeof(msg) - 1)

static void login_trilogy_handle_op_version(LoginClient* client, ProtocolHandler* handler)
{
    Basic* basic            = protocol_handler_basic(handler);
    PacketTrilogy* packet   = packet_trilogy_create(basic, TrilogyOp_Version, sizeof(EQP_LOGIN_TRILOGY_VERSION));
    Aligned w;
    
    aligned_init(basic, &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    aligned_write_buffer(&w, EQP_LOGIN_TRILOGY_VERSION, sizeof(EQP_LOGIN_TRILOGY_VERSION));
    
    login_trilogy_schedule_packet(handler, packet);
    
    login_client_set_state(client, LoginClientTrilogy_VersionSent);
}

static void login_trilogy_handle_op_credentials(LoginClient* client, ProtocolHandler* handler, Aligned* a)
{
    Login* login;
    LoginCrypto* crypto;
    LoginTrilogy_Credentials* cred;
    int nameLength;
    int passLength;
    
    if (login_client_get_state(client) != LoginClientTrilogy_VersionSent || aligned_remaining(a) < sizeof(LoginTrilogy_Credentials))
        return;
    
    login_client_set_state(client, LoginClientTrilogy_ProcessingCredentials);
    
    login   = (Login*)protocol_handler_basic(handler); // Legal! We know this will always be the login object here
    crypto  = login_get_crypto(login);
    
    login_crypto_decrypt_trilogy(crypto, aligned_current(a), sizeof(LoginTrilogy_Credentials));
    cred = login_crypto_data_type(crypto, LoginTrilogy_Credentials);
    
    nameLength = strlen(cred->username);
    passLength = strlen(cred->password);
    
    if ((uint32_t)nameLength > (sizeof(cred->username) - 1) || (uint32_t)passLength > (sizeof(cred->password) - 1))
    {
        login_trilogy_err_literal(handler, ERR_CREDENTIALS);
        return;
    }
    
    login_client_check_credentials(client, login, cred->username, nameLength, cred->password, passLength);
    login_crypto_clear(crypto);
}

void login_client_trilogy_handle_credentials_result(LoginClient* client, uint32_t accountId)
{
    ProtocolHandler* handler = login_client_handler(client);
    
    if (accountId == 0)
    {
        login_trilogy_err_literal(handler, ERR_CREDENTIALS);
    }
    else
    {
        Basic* basic            = protocol_handler_basic(handler);
        PacketTrilogy* packet   = packet_trilogy_create_type(basic, TrilogyOp_Session, LoginTrilogy_Session);
        Aligned write;
        Aligned* w = &write;
        
        aligned_init(basic, w, packet_trilogy_data(packet), packet_trilogy_length(packet));
        
        // sessionId
        aligned_write_snprintf_full_advance(w, sizeof_field(LoginTrilogy_Session, sessionId), "LS#%u", accountId);
        // "unused"
        aligned_write_literal_null_terminated(w, "unused");
        // unknown
        aligned_write_uint32(w, 4);
        
        login_trilogy_schedule_packet(handler, packet);
        login_client_set_state(client, LoginClientTrilogy_AcceptedCredentials);
    }
}

static void login_trilogy_handle_op_banner(LoginClient* client, ProtocolHandler* handler)
{
    Basic* basic;
    PacketTrilogy* packet;
    Aligned w;
    
    if (login_client_get_state(client) != LoginClientTrilogy_AcceptedCredentials)
        return;
    
    basic   = protocol_handler_basic(handler);
    packet  = packet_trilogy_create(basic, TrilogyOp_Banner, sizeof(uint32_t) + sizeof(EQP_LOGIN_TRILOGY_BANNER_MESSAGE));
    
    aligned_init(basic, &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    // unknown
    aligned_write_uint32(&w, 1);
    // message
    aligned_write_literal_null_terminated(&w, EQP_LOGIN_TRILOGY_BANNER_MESSAGE);
    
    login_trilogy_schedule_packet(handler, packet);
}

static void login_trilogy_handle_op_server_list(LoginClient* client, ProtocolHandler* handler)
{
    Login* login;
    ServerList* list;
    ServerListing* data;
    PacketTrilogy* packet;
    Aligned write;
    Aligned* w  = &write;
    uint32_t count;
    uint32_t length;
    uint32_t n;
    uint32_t i;
    uint32_t ip;
    bool isLocal;
    char address[INET_ADDRSTRLEN];
    
    if (login_client_get_state(client) != LoginClientTrilogy_AcceptedCredentials)
        return;
    
    isLocal = login_client_is_local(client);
    login   = (Login*)protocol_handler_basic(handler);
    list    = login_server_list(login);
    count   = server_list_count(list);
    ip      = protocol_handler_ip_address(handler)->sin_addr.s_addr;
    
    snprintf(address, sizeof(address), "%u.%u.%u.%u", (ip >> 0) & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
    
    // Figure out how long the packet will be
    data    = server_list_data(list);
    length  = sizeof(LoginTrilogy_ServerListHeader) + sizeof(LoginTrilogy_ServerListFooter);
    n       = 0;
    
    for (i = 0; i < count; i++)
    {
        ServerListing* server = &data[i];
        
        if (!server_listing_name(server))
            continue;
        
        /*
            There are two different 'local address' cases to handle:
            
            1) Login, char-select and client are all local to each other; or
            2) Char-select and client are local to each other, but both are remote to login
            
            In the first case, both char-select and client will have been flagged as local (to the login server) and the
            client's address will not match the remote address for char-select.
            
            In the second case, neither char-select and client will have been flagged as local, but the client's address will
            (presumably, and typically) be the same as char-select's remote address.
        */
        if ((isLocal && server_listing_is_local(server)) || string_compare_cstr(server_listing_remote_address(server), address))
            length += string_length(server_listing_local_address(server)) + 1;
        else
            length += server_listing_remote_length(server);
        
        length += server_listing_name_length(server);
        length += sizeof(LoginTrilogy_ServerFooter);
        
        if (++n == SERVER_LIST_COUNT_MAX)
            break;
    }
    
    // Create and fill the packet
    packet = packet_trilogy_create(B(login), TrilogyOp_ServerList, length);
    aligned_init(B(login), w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    /* ServerListHeader */
    // serverCount
    aligned_write_uint8(w, n);
    // unknown (2 bytes)
    aligned_write_zeroes(w, sizeof(uint8_t) * 2);
    // showNumPlayers
    aligned_write_uint8(w, 0xff); // Show actual numbers rather than just "Up"
    
    /* Server entries and footers */
    n = 0;
    
    for (i = 0; i < count; i++)
    {
        int playerCount;
        ServerListing* server = &data[i];
        
        if (!server_listing_name(server))
            continue;
        
        switch (server_listing_status(server))
        {
        case ServerStatus_Down:
            playerCount = SERVER_DOWN;
            break;
        
        case ServerStatus_Locked:
            playerCount = SERVER_LOCKED;
            break;
        
        default:
            playerCount = server_listing_player_count(server);
            break;
        }
        
        // server name
        aligned_write_string_null_terminated(w, server_listing_name(server));
        // ip address
        if ((isLocal && server_listing_is_local(server)) || string_compare_cstr(server_listing_remote_address(server), address))
            aligned_write_string_null_terminated(w, server_listing_local_address(server));
        else
            aligned_write_string_null_terminated(w, server_listing_remote_address(server));
        // isGreenName
        aligned_write_bool(w, (server_listing_rank(server) != ServerRank_Standard));
        // playerCount
        aligned_write_int32(w, playerCount);
        
        if (++n == SERVER_LIST_COUNT_MAX)
            break;
    }
    
    /* ServerListFooter */
    // admin
    aligned_write_uint8(w, 0);
    // zeroes A (8 bytes)
    aligned_write_zeroes(w, sizeof(uint8_t) * 8);
    // kunark
    aligned_write_uint8(w, 1);
    // velious
    aligned_write_uint8(w, 1);
    // zeroes B (12 bytes)
    aligned_write_zeroes(w, sizeof(uint8_t) * 12);
    
    login_trilogy_schedule_packet(handler, packet);
}

static void login_trilogy_handle_op_server_status_request(LoginClient* client, ProtocolHandler* handler, Aligned* a)
{
    const char* ipAddress = (const char*)aligned_current(a);
    
    if (login_client_get_state(client) != LoginClientTrilogy_AcceptedCredentials)
        return;
    
    server_list_send_client_login_request_by_ip_address((Login*)protocol_handler_basic(handler), ipAddress, login_client_account_id(client));
}

void login_client_trilogy_handle_login_response(LoginClient* client, int response)
{
    ProtocolHandler* handler;
    
    if (login_client_get_state(client) != LoginClientTrilogy_AcceptedCredentials)
        return;
    
    handler = login_client_handler(client);
    
    switch (response)
    {
    case LOGIN_RESPONSE_ACCEPTED:
        login_trilogy_schedule_packet(handler, packet_trilogy_create(protocol_handler_basic(handler), TrilogyOp_ServerStatusAccept, 0));
        break;
    
    default:
    case LOGIN_RESPONSE_DENIED:
        login_trilogy_err_literal(handler, "Error: Your login request was denied");
        break;
    
    case LOGIN_RESPONSE_SUSPENDED:
        login_trilogy_err_literal(handler, "Error: Your account has been suspended from the selected server");
        break;
    
    case LOGIN_RESPONSE_BANNED:
        login_trilogy_err_literal(handler, "Error: Your account has been banned from the selected server");
        break;
    
    case LOGIN_RESPONSE_WORLD_FULL:
        login_trilogy_err_literal(handler, "Error: The selected server is currently full");
        break;
    }
}

static void login_trilogy_handle_op_session_key(LoginClient* client, ProtocolHandler* handler)
{
    Basic* basic;
    PacketTrilogy* packet;
    Aligned write;
    Aligned* w = &write;
    
    if (login_client_get_state(client) != LoginClientTrilogy_AcceptedCredentials)
        return;

    basic   = protocol_handler_basic(handler);
    packet  = packet_trilogy_create(basic, TrilogyOp_SessionKey, sizeof_field(LoginClient, sessionKey) + 1);
    
    aligned_init(basic, w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    aligned_write_uint8(w, 0);
    aligned_write_buffer(w, login_client_session_key(client), sizeof_field(LoginClient, sessionKey) - 1);
    aligned_write_uint8(w, 0);
    
    login_trilogy_schedule_packet(handler, packet);
}

void client_recv_packet_trilogy(void* vclient, uint16_t opcode, Aligned* a)
{
    LoginClient* client         = (LoginClient*)vclient;
    ProtocolHandler* handler    = login_client_handler(client);

    switch (opcode)
    {
    case TrilogyOp_Version:
        login_trilogy_handle_op_version(client, handler);
        break;
    
    case TrilogyOp_Credentials:
        login_trilogy_handle_op_credentials(client, handler, a);
        break;
    
    case TrilogyOp_Banner:
        login_trilogy_handle_op_banner(client, handler);
        break;
    
    case TrilogyOp_ServerList:
        login_trilogy_handle_op_server_list(client, handler);
        break;
    
    case TrilogyOp_ServerStatusRequest:
        login_trilogy_handle_op_server_status_request(client, handler, a);
        break;
    
    case TrilogyOp_SessionKey:
        login_trilogy_handle_op_session_key(client, handler);
        break;
    
    default:
        break;
    }
}

#undef ERR_CREDENTIALS
#undef SERVER_LIST_COUNT_MAX
#undef SERVER_DOWN
#undef SERVER_LOCKED
