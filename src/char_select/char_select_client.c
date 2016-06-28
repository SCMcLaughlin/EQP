
#include "char_select_client.h"
#include "char_select_client_trilogy.h"
#include "eqp_char_select.h"

CharSelectClient* char_select_client_create(R(ProtocolHandler*) handler, int expansion)
{
    CharSelectClient* client = eqp_alloc_type(protocol_handler_basic(handler), CharSelectClient);
    
    memset(client, 0, sizeof(CharSelectClient));
    
    client->expansion   = expansion;
    client->handler     = handler;
    atomic_init(&client->refCount, 1);
    
    protocol_handler_grab(handler);
    
    return client;
}

void char_select_client_drop(R(CharSelectClient*) client)
{
    if (atomic_fetch_sub(&client->refCount, 1) > 1)
        return;
    
    printf("Client object destroyed\n");
    char_select_remove_client_from_unauthed_list((CharSelect*)protocol_handler_basic(client->handler), client);
    protocol_handler_drop(client->handler);
    free(client);
}

void client_on_disconnect(R(void*) vclient, int isLinkdead)
{
    R(CharSelectClient*) client = (CharSelectClient*)vclient;
    
    if (client)
        char_select_client_drop(client);
    
    printf("DISCONNECTED (%s)\n", isLinkdead ? "timeout" : "explicit");
}

void char_select_client_set_auth(R(CharSelectClient*) client, R(CharSelectAuth*) auth)
{
    client->auth = *auth;
    
    if (client->expansion == ExpansionId_Trilogy)
        cs_client_trilogy_on_auth(client);
    //else
    //    cs_client_standard_on_auth(client);
}

static void char_select_client_insert_account_id_callback(R(Query*) query)
{
    R(CharSelectClient*) client = query_userdata_type(query, CharSelectClient);
    R(Basic*) basic             = B(protocol_handler_basic(client->handler));
    R(Database*) db             = core_db(C(basic));
    int64_t accountId           = query_last_insert_id(query);
    Query q;
    
    log_format(basic, LogInfo, "Created new account with id %li for account '%s' with login server id %u", accountId,
        client->auth.accountName, client->auth.accountId);
    
    client->auth.accountId = (uint32_t)accountId;
    
    if (client->expansion == ExpansionId_Trilogy)
        cs_client_trilogy_on_account_id(client, (uint32_t)accountId);
    //else
    //    cs_client_standard_on_account_id(client, (uint32_t)accountId);
    
    // Drop this query's reference to the client
    char_select_client_drop(client);
    
    query_init(&q);
    db_prepare_literal(db, &q, "INSERT INTO account (name_id_pair) VALUES (?)", NULL);
    
    query_bind_int64(&q, 1, accountId);
    
    db_schedule(db, &q);
}

static void char_select_client_query_account_id_callback(R(Query*) query)
{
    R(CharSelectClient*) client = query_userdata_type(query, CharSelectClient);
    R(Database*) db;
    Query q;
    
    while (query_select(query))
    {
        uint32_t accountId      = (uint32_t)query_get_int64(query, 1);
        client->auth.accountId  = accountId;
        
        if (client->expansion == ExpansionId_Trilogy)
            cs_client_trilogy_on_account_id(client, accountId);
        //else
        //    cs_client_standard_on_account_id(client, accountId);
        
        // Drop this query's reference to the client
        char_select_client_drop(client);
        return;
    }
    
    // If we reach here, carry the client's current ref count forward
    
    db = core_db(C(protocol_handler_basic(client->handler)));
    
    // We did not have an entry for this account id + name combination; create a new one
    query_init(&q);
    query_set_userdata(&q, client);
    db_prepare_literal(db, &q, "INSERT INTO account_name_id_pair (id, name) VALUES (?, ?)", char_select_client_insert_account_id_callback);
    
    query_bind_int64(&q, 1, (int64_t)client->auth.accountId);
    query_bind_string(&q, 2, client->auth.accountName, -1);
    
    db_schedule(db, &q);
}

void char_select_client_query_account_id(R(CharSelectClient*) client, R(CharSelect*) charSelect)
{
    Query query;
    
    char_select_client_grab(client);
    
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(core_db(C(charSelect)), &query, "SELECT rowid FROM account_name_id_pair WHERE id = ? AND name = ?", char_select_client_query_account_id_callback);
    
    query_bind_int64(&query, 1, (int64_t)client->auth.accountId);
    query_bind_string(&query, 2, client->auth.accountName, -1);
    
    db_schedule(core_db(C(charSelect)), &query);
}

static void char_select_client_query_character_name_callback(R(Query*) query)
{
    R(CharSelectClient*) client = query_userdata_type(query, CharSelectClient);
    int taken = false;
    
    while (query_select(query))
    {
        taken = true;
    }
    
    if (client->expansion == ExpansionId_Trilogy)
        cs_client_trilogy_on_character_name_checked(client, taken);
    //else
    //    cs_client_standard_on_character_name_checked(client, taken);
    
    char_select_client_drop(client);
}

void char_select_client_query_character_name_taken(R(CharSelectClient*) client, R(CharSelect*) charSelect, R(const char*) name)
{
    Query query;
    
    char_select_client_grab(client);
    
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(core_db(C(charSelect)), &query, "SELECT 1 FROM character WHERE name = ?", char_select_client_query_character_name_callback);
    
    query_bind_string(&query, 1, name, -1);
    
    db_schedule(core_db(C(charSelect)), &query);
}

static void char_select_client_delete_character_callback(R(Query*) query)
{
    R(CharSelectClient*) client = query_userdata_type(query, CharSelectClient);
    
    if (query_affected_rows(query) != 0)
    {
        uint32_t accountId = char_select_client_account_id(client);
        
        // on_account_id() is the trigger for retrieving and sending the up-to-date char select info
        if (client->expansion == ExpansionId_Trilogy)
            cs_client_trilogy_on_account_id(client, accountId);
        //else
        //    cs_client_standard_on_account_id(client, accountId);
    }
    
    char_select_client_drop(client);
}

void char_select_client_delete_character_by_name(R(CharSelectClient*) client, R(CharSelect*) charSelect, R(const char*) name)
{
    Query query;
    
    char_select_client_grab(client);
    
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(core_db(C(charSelect)), &query, "DELETE FROM character WHERE name_id_pair = ? AND name = ?", char_select_client_delete_character_callback);
    
    //fixme: also need to delete anything related to this character...
    
    query_bind_int64(&query, 1, (int64_t)char_select_client_account_id(client));
    query_bind_string(&query, 2, name, -1);
    
    db_schedule(core_db(C(charSelect)), &query);
}

void char_select_client_on_zone_in_failure(R(CharSelectClient*) client, R(CharSelect*) charSelect, R(const char*) zoneShortName)
{
    if (client->expansion == ExpansionId_Trilogy)
        cs_client_trilogy_on_zone_in_failure(client, charSelect, zoneShortName);
    //else
    //    cs_client_standard_on_zone_in_failure(client, charSelect, zoneShortName);
}

void char_select_client_on_zone_in_success(R(CharSelectClient*) client, R(CharSelect*) charSelect, R(Server_ZoneAddress*) zoneAddr)
{
    if (client->expansion == ExpansionId_Trilogy)
        cs_client_trilogy_on_zone_in_success(client, charSelect, zoneAddr);
    //else
    //    cs_client_standard_on_zone_in_success(client, charSelect, zoneAddr);
}
