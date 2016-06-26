
#include "client.h"
#include "zone.h"
#include "zone_cluster.h"

static Client* client_create_unidentified_stub(R(ProtocolHandler*) handler, int expansion)
{
    R(Basic*) basic     = protocol_handler_basic(handler);
    R(Client*) client   = eqp_alloc_type(basic, Client);
    
    memset(client, 0, sizeof(Client));
    
    protocol_handler_grab(handler);
    
    client->handler         = handler;
    client->expansion       = expansion;
    atomic_init(&client->refCount, 1);
    client->isStubClient    = true;
    
    return client;
}

void* client_create_from_new_connection_standard(R(ProtocolHandler*) handler)
{
    return client_create_unidentified_stub(handler, ExpansionId_Unknown);
}

void* client_create_from_new_connection_trilogy(R(ProtocolHandler*) handler)
{
    return client_create_unidentified_stub(handler, ExpansionId_Trilogy);
}

void client_on_disconnect(R(void*) vclient, int isLinkdead)
{
    R(Client*) client = (Client*)vclient;
    
    client_drop(client);
    
    printf("DISCONNECT (%s)\n", isLinkdead ? "explicit" : "timeout");
}

static void client_load_stats_callback(R(Query*) query)
{
    R(Client*) client = query_userdata_type(query, Client);
    
    while (query_select(query))
    {
        
    }
    
    client->loaded.stats = true;
    
    if (client->loaded.connection)
    {
        //send packet
        client_check_loading_finished(client);
    }
    
    client_drop(client);
}

static void client_load_inventory_callback(R(Query*) query)
{
    R(Client*) client   = query_userdata_type(query, Client);
    R(Basic*) basic     = B(client_zone_cluster(client));
    R(Inventory*) inv   = &client->inventory;
    InventorySlot slot;
    
    while (query_select(query))
    {
        slot.slotId         = query_get_int(query, 1);
        slot.augSlotId      = query_get_int(query, 2);
        slot.stackAmount    = query_get_int(query, 3);
        slot.charges        = query_get_int(query, 4);
        slot.itemId         = query_get_int(query, 5);
        
        //fixme: create Item object with pointer from shared memory region...
        
        inventory_add_from_database(basic, inv, &slot);
    }
    
    client->loaded.inventory = true;
    
    if (client->loaded.connection)
    {
        //send packet
        client_check_loading_finished(client);
    }
    
    client_drop(client);
}

static void client_load_skills_callback(R(Query*) query)
{
    R(Client*) client   = query_userdata_type(query, Client);
    R(Skills*) skills   = &client->skills;
    
    while (query_select(query))
    {
        uint32_t skillId    = query_get_int(query, 1);
        uint32_t value      = query_get_int(query, 2);
        
        skills_set_from_db(skills, skillId, value);
    }
    
    client->loaded.skills = true;
    
    if (client->loaded.connection)
    {
        //send packet
        client_check_loading_finished(client);
    }
    
    client_drop(client);
}

Client* client_create(R(ZC*) zc, R(Zone*) zone, R(Server_ClientZoning*) zoning)
{
    R(Database*) db = core_db(C(zc));
    Client* client  = eqp_alloc_type(B(zc), Client);
    Query query;
    
    memset(client, 0, sizeof(Client));
    
    atomic_init(&client->refCount, 1);
    mob_init_client(&client->mob, zc, zone, zoning);
    inventory_init(B(zc), &client->inventory);
    skills_init(&client->skills);
    
    client->isLocal     = zoning->isLocal;
    client->characterId = zoning->characterId;
    client->accountName = string_create_from_cstr(B(zc), zoning->accountName, strlen(zoning->accountName));
    client->accountId   = zoning->accountId;
    client->ipAddress   = zoning->ipAddress;
    
    // Character stats
    client_grab(client);
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(db, &query,
        "SELECT "
            "surname, level, class, race, gender, face, deity, x, y, z, current_hp, current_mana, current_endurance, experience, "
            "base_str, base_sta, base_dex, base_agi, base_int, base_wis, base_cha, fk_guild_id, guild_rank "
        "FROM character WHERE character_id = ?", client_load_stats_callback);
    query_bind_int64(&query, 1, zoning->characterId);
    db_schedule(db, &query);
    
    // Inventory
    client_grab(client);
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(db, &query, 
        "SELECT slot_id, aug_slot_id, stack_amount, charges, item_id FROM inventory "
        "WHERE character_id = ? "
        "ORDER BY slot_id ASC", // Putting lower slots first will be a decent micro-optimization in the average case
        client_load_inventory_callback);
    query_bind_int64(&query, 1, zoning->characterId);
    db_schedule(db, &query);
    
    // Skills
    client_grab(client);
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(db, &query, "SELECT skillId, value FROM skills WHERE character_id = ?", client_load_skills_callback);
    query_bind_int64(&query, 1, zoning->characterId);
    db_schedule(db, &query);
    
    return client;
}

void client_drop(R(Client*) client)
{
    if (atomic_fetch_sub(&client->refCount, 1) > 1)
        return;
    
    if (client->isStubClient)
        goto stub;
    
    inventory_deinit(&client->inventory);
    mob_deinit(&client->mob);
    
stub:
    protocol_handler_drop(client_handler(client));
    free(client);
}

void client_catch_up_with_loading_progress(R(Client*) client)
{
    // This is called when the client is matched with its UDP connection;
    // stuff has already been loading for this client in the background,
    // so check if we should send some packets.
    
    client->loaded.connection = true;
    
    if (client->loaded.stats)
    {
        
    }
    
    if (client->loaded.inventory)
    {
        
    }
    
    client_check_loading_finished(client);
}

void client_check_loading_finished(R(Client*) client)
{
    R(ZC*) zc;
    
    if (!client->loaded.connection || !client->loaded.stats || !client->loaded.inventory || !client->loaded.skills)
        return;
    
    zc = client_zone_cluster(client);
    (void)zc;
}
