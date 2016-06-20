
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
    db_prepare_literal(db, &q, "INSERT INTO account (fk_name_id_pair) VALUES (?)", NULL);
    
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
