
#include "login_client_trilogy.h"
#include "eqp_login.h"

#define ERR_CREDENTIALS "Error: Invalid username/password"

void* client_create_from_new_connection_trilogy(R(ProtocolHandler*) handler)
{
    return login_client_create(handler, 0, LoginClientTrilogy_BrandNew);
}

static void login_trilogy_err_msg(R(ProtocolHandler*) handler, R(const char*) msg, uint32_t len)
{
    R(Basic*) basic             = protocol_handler_basic(handler);
    R(PacketTrilogy*) packet    = packet_trilogy_create(basic, TrilogyOp_Error, ++len);
    Aligned w;
    
    aligned_init(basic, &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    aligned_write_buffer(&w, msg, len);
    
    protocol_handler_trilogy_schedule_packet(&handler->trilogy, packet);
}

#define login_trilogy_err_literal(handler, msg) login_trilogy_err_msg((handler), (msg), sizeof(msg) - 1)

static void login_trilogy_handle_op_version(R(LoginClient*) client, R(ProtocolHandler*) handler)
{
    R(Basic*) basic             = protocol_handler_basic(handler);
    R(PacketTrilogy*) packet    = packet_trilogy_create(basic, TrilogyOp_Version, sizeof(EQP_LOGIN_TRILOGY_VERSION));
    Aligned w;
    
    aligned_init(basic, &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    aligned_write_buffer(&w, EQP_LOGIN_TRILOGY_VERSION, sizeof(EQP_LOGIN_TRILOGY_VERSION));
    
    protocol_handler_trilogy_schedule_packet(&handler->trilogy, packet);
    
    login_client_set_state(client, LoginClientTrilogy_VersionSent);
}

static void login_trilogy_credentials_callback(R(Query*) query)
{
    R(LoginClient*) client      = query_userdata_type(query, LoginClient);
    R(ProtocolHandler*) handler = login_client_handler(client);
    R(const char*) password     = login_client_password_temp(client);
    int passLength              = login_client_password_length(client);
    R(Login*) login             = (Login*)protocol_handler_basic(handler);
    R(LoginCrypto*) crypto      = login_get_crypto(login);
    int success                 = false;
    
    while (query_select(query))
    {
        uint32_t hashLength;
        uint32_t saltLength;
        int64_t id          = query_get_int64(query, 1);
        R(const byte*) hash = query_get_blob(query, 2, &hashLength);
        R(const byte*) salt = query_get_blob(query, 3, &saltLength);
        
        login_crypto_hash(crypto, password, passLength, salt, saltLength);
        
        if (memcmp(login_crypto_data(crypto), hash, hashLength) == 0)
        {
            // Successful login
            R(PacketTrilogy*) packet = packet_trilogy_create_type(B(login), TrilogyOp_Session, LoginTrilogy_Session);
            Aligned write;
            R(Aligned*) w = &write;
            
            aligned_init(B(login), w, packet_trilogy_data(packet), packet_trilogy_length(packet));
            
            // sessionId
            aligned_write_snprintf_full_advance(w, sizeof_field(LoginTrilogy_Session, sessionId), "LS#%u", (uint32_t)id);
            // "unused"
            aligned_write_literal_null_terminated(w, "unused");
            // unknown
            aligned_write_uint32(w, 4);
            
            protocol_handler_trilogy_schedule_packet(&handler->trilogy, packet);
            
            success = true;
            login_client_set_state(client, LoginClientTrilogy_AcceptedCredentials);
        }
    }
    
    login_crypto_clear(crypto);
    login_client_clear_password_temp(client);
    
    if (!success)
        login_trilogy_err_literal(handler, ERR_CREDENTIALS);
}

static void login_trilogy_handle_op_credentials(R(LoginClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    int state = login_client_get_state(client);
    R(Login*) login;
    R(LoginCrypto*) crypto;
    R(PacketTrilogy*) packet;
    R(LoginTrilogy_Credentials*) cred;
    int nameLength;
    int passLength;
    Query query;
    Aligned w;
    
    if (state != LoginClientTrilogy_VersionSent || aligned_remaining(a) < sizeof(LoginTrilogy_Credentials))
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
    
    printf("username: %s, password: %s\n", cred->username, cred->password);
    
    login_client_set_password_temp(client, cred->password, passLength);
    
    // Prep database query
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(core_db(C(login)), &query, "SELECT rowid, password, salt FROM local_login WHERE username = ?", login_trilogy_credentials_callback);
    
    query_bind_string(&query, 1, cred->username, nameLength);
    
    db_schedule(core_db(C(login)), &query);
    login_crypto_clear(crypto);
}

void client_recv_packet_trilogy(R(void*) vclient, uint16_t opcode, R(Aligned*) a)
{
    R(LoginClient*) client      = (LoginClient*)vclient;
    R(ProtocolHandler*) handler = login_client_handler(client);

    printf("Received packet opcode 0x%04x, length %u:\n", opcode, aligned_remaining(a));
    
    switch (opcode)
    {
    case TrilogyOp_Version:
        login_trilogy_handle_op_version(client, handler);
        break;
    
    case TrilogyOp_Credentials:
        login_trilogy_handle_op_credentials(client, handler, a);
        break;
    
    default:
        break;
    }
}

#undef ERR_CREDENTIALS
