
#include "login_client.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"
#include "client_list.h"
#include "eqp_login.h"
#include "login_client_trilogy.h"

LoginClient* login_client_create(R(ProtocolHandler*) handler, int expansion, int state)
{
    LoginClient* client = eqp_alloc_type(protocol_handler_basic(handler), LoginClient);
    
    client->state       = state;
    client->expansion   = expansion;
    client->handler     = handler;
    atomic_init(&client->refCount, 1);
    client->accountName = NULL;
    client->accountId   = 0;
    client->isLocal     = login_is_ip_address_local(protocol_handler_ip_address(handler)->sin_addr.s_addr);
    
    protocol_handler_grab(handler);
    
    return client;
}

void login_client_drop(R(LoginClient*) client)
{
    R(ClientList*) clientList;
    
    if (atomic_fetch_sub(&client->refCount, 1) > 1)
        return;
    
    clientList = login_client_list((Login*)protocol_handler_basic(client->handler));
    client_list_remove(clientList, client);
    
    if (client->accountName)
        string_destroy(client->accountName);
    
    protocol_handler_drop(client->handler);
    free(client);
}

void client_on_disconnect(R(void*) vclient, int isLinkdead)
{
    R(LoginClient*) client = (LoginClient*)vclient;
    (void)isLinkdead;
    
    if (client)
        login_client_drop(client);
}

void login_client_set_account_name(R(LoginClient*) client, R(const char*) name, int length)
{
    client->accountName = string_create_from_cstr(protocol_handler_basic(client->handler), name, length);
}

void login_client_set_password_temp(R(LoginClient*) client, R(const char*) password, int length)
{
    if ((uint32_t)length >= sizeof(client->passwordTemp))
        length = sizeof(client->passwordTemp) - 1;
    
    memcpy(client->passwordTemp, password, length);
    client->passwordTemp[length] = 0;
    client->passwordLength = length;
}

#ifndef EQP_LOGIN_DISABLE_ACCOUNT_AUTO_CREATION
static void login_client_auto_create_account_callback(R(Query*) query)
{
    R(LoginClient*) client  = query_userdata_type(query, LoginClient);
    R(Login*) login         = (Login*)protocol_handler_basic(login_client_handler(client));
    uint32_t accountId      = (uint32_t)query_last_insert_id(query);
    
    login_client_set_account_id(client, accountId);
    client_list_add(B(login), login_client_list(login), client);
    
    log_format(B(login), LogInfo, "Created new login account '%s' with id %u", string_data(login_client_account_name(client)), accountId);
    
    if (client->expansion == ExpansionId_Trilogy)
        login_client_trilogy_handle_credentials_result(client, accountId);
    /*else
        login_client_standard_handle_credentials_result(client, accountId);*/
    
    login_client_drop(client);
}

static void login_client_auto_create_account(R(LoginClient*) client)
{
    R(ProtocolHandler*) handler = login_client_handler(client);
    R(String*) username         = login_client_account_name(client);
    R(const char*) password     = login_client_password_temp(client);
    int passLength              = login_client_password_length(client);
    R(Login*) login             = (Login*)protocol_handler_basic(handler);
    R(LoginCrypto*) crypto      = login_get_crypto(login);
    byte salt[EQP_LOGIN_SALT_LENGTH];
    Query query;
    
    random_bytes(salt, EQP_LOGIN_SALT_LENGTH);
    login_crypto_hash(crypto, password, passLength, salt, EQP_LOGIN_SALT_LENGTH);
    
    login_client_clear_password_temp(client);
    
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(core_db(C(login)), &query, "INSERT INTO login (username, password, salt) VALUES (?, ?, ?)", login_client_auto_create_account_callback);
    
    query_bind_string(&query, 1, string_data(username), string_length(username));
    query_bind_blob(&query, 2, login_crypto_data(crypto), EQP_LOGIN_CRYPTO_HASH_SIZE);
    query_bind_blob(&query, 3, salt, EQP_LOGIN_SALT_LENGTH);
    
    db_schedule(core_db(C(login)), &query);
    
    login_crypto_clear(crypto);
}
#endif

static void login_client_credentials_callback(R(Query*) query)
{
    R(LoginClient*) client      = query_userdata_type(query, LoginClient);
    R(ProtocolHandler*) handler = login_client_handler(client);
    R(const char*) password     = login_client_password_temp(client);
    int passLength              = login_client_password_length(client);
    R(Login*) login             = (Login*)protocol_handler_basic(handler);
    R(LoginCrypto*) crypto      = login_get_crypto(login);
    int success                 = 0;
    
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
            if (client->expansion == ExpansionId_Trilogy)
                login_client_trilogy_handle_credentials_result(client, (uint32_t)id);
            /*else
                login_client_standard_handle_credentials_result(client, (uint32_t)id);*/
            
            success = 1;
            login_client_set_account_id(client, (uint32_t)id);
            client_list_add(B(login), login_client_list(login), client);
        }
#ifndef EQP_LOGIN_DISABLE_ACCOUNT_AUTO_CREATION
        else
        {
            success = -1; // If -1, the account already exists and we failed to log into it
        }
#endif
        
        login_crypto_clear(crypto);
    }
    
#ifndef EQP_LOGIN_DISABLE_ACCOUNT_AUTO_CREATION
    if (success == 0)
    {
        // Carry forward the current ref count for the auto-create query
        login_client_auto_create_account(client);
        return;
    }
#endif
    
    login_client_clear_password_temp(client);
    
    if (success != 1)
    {
        if (client->expansion == ExpansionId_Trilogy)
            login_client_trilogy_handle_credentials_result(client, 0);
        /*else
            login_client_standard_handle_credentials_result(client, 0);*/
    }
    
    // Drop this query's reference to the client
    login_client_drop(client);
}

void login_client_check_credentials(R(LoginClient*) client, R(Login*) login, R(const char*) username, int nameLength,
    R(const char*) password, int passLength)
{
    Query query;
    
    login_client_grab(client);
    
    login_client_set_account_name(client, username, nameLength);
    login_client_set_password_temp(client, password, passLength);
    
    // Prep database query
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(core_db(C(login)), &query, "SELECT rowid, password, salt FROM login WHERE username = ?", login_client_credentials_callback);
    
    query_bind_string(&query, 1, username, nameLength);
    
    db_schedule(core_db(C(login)), &query);
}

void login_client_handle_login_response(R(LoginClient*) client, int response)
{
    if (client->expansion == ExpansionId_Trilogy)
        login_client_trilogy_handle_login_response(client, response);
    //else
    //  login_client_standard_handle_login_response(client, response);
}

void login_client_generate_session_key(R(LoginClient*) client)
{
    R(char*) key    = client->sessionKey;
    uint32_t n      = sizeof(client->sessionKey) - 1;
    uint32_t i;
    
    for (i = 0; i < n; i++)
    {
        static const char keys[] =
        {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
            'Y', 'Z', '0', '1', '2', '3', '4', '5',
            '6', '7', '8', '9'
        };
        
        key[i] = keys[random_uint32() % sizeof(keys)];
    }
    
    key[n] = 0;
}
