
#include "client.h"
#include "zone.h"
#include "zone_cluster.h"

static Client* client_create_unidentified_stub(R(ProtocolHandler*) handler, int expansion)
{
    R(Basic*) basic     = protocol_handler_basic(handler);
    R(Client*) client   = eqp_alloc_type(basic, Client);
    
    memset(client, 0, sizeof(Client));
    
    protocol_handler_grab(handler);
    
    client->handler             = handler;
    client->expansion           = expansion;
    atomic_init(&client->refCount, 1);
    client->isStubClient        = true;
    client->creationTimestamp   = clock_milliseconds();
    
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
    
    printf("DISCONNECT (%s)\n", isLinkdead ? "timeout" : "explicit");
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
        client_check_loading_finished(client);
    
    client_drop(client);
}

static void client_load_stats_callback(R(Query*) query)
{
    R(Client*) client   = query_userdata_type(query, Client);
    R(Mob*) mob         = &client->mob;
    R(ZC*) zc           = client_zone_cluster(client);
    R(Database*) db;
    Query q;
    
    while (query_select(query))
    {
        int val;
        
        // surname
        if (!query_is_null(query, 1))
        {
            uint32_t len;
            R(const char*) surname  = query_get_string(query, 1, &len);
            client->surname         = string_create_from_cstr(B(zc), surname, len);
        }
        
        // level
        mob->level                  = query_get_int(query, 2); //fixme: validate later (we don't know expansion to put a cap on it yet)
        // class
        mob->class                  = query_get_int(query, 3); //fixme: validate later
        // race
        val                         = query_get_int(query, 4);
        mob->baseRace               = val;
        mob->currentRace            = val;
        // gender
        val                         = query_get_int(query, 5);
        mob->baseGender             = val;
        mob->currentGender          = val;
        // face
        mob->face                   = query_get_int(query, 6);
        // deity
        mob->deity                  = query_get_int(query, 7);
        // x
        mob->x                      = query_get_double(query, 8);
        // y
        mob->y                      = query_get_double(query, 9);
        // z
        mob->z                      = query_get_double(query, 10);
        // heading
        mob->heading                = query_get_double(query, 11);
        // currentHp
        mob->currentHp              = query_get_int64(query, 12);
        // currentMana
        mob->currentMana            = query_get_int64(query, 13);
        // currentEndurance
        mob->currentEndurance       = query_get_int64(query, 14);
        // experience
        client->experience          = query_get_int64(query, 15);
        // trainingPoints
        client->trainingPoints      = query_get_int(query, 16);
        // baseStr
        mob_set_base_str(mob, query_get_int(query, 17));
        // baseSta
        mob_set_base_sta(mob, query_get_int(query, 18));
        // baseDex
        mob_set_base_dex(mob, query_get_int(query, 19));
        // baseAgi
        mob_set_base_agi(mob, query_get_int(query, 20));
        // baseInt
        mob_set_base_int(mob, query_get_int(query, 21));
        // baseWis
        mob_set_base_wis(mob, query_get_int(query, 22));
        // baseCha
        mob_set_base_cha(mob, query_get_int(query, 23));
        // guildId
        client->guildId             = query_get_int(query, 24);
        // guildRank
        client->guildRank           = query_get_int(query, 25);
        // harmtouchTimestamp
        client->harmtouchTimestamp  = query_get_int64(query, 26);
        // disciplineTimestamp
        client->disciplineTimestamp = query_get_int64(query, 27);
        // pp
        client->coins.pp            = query_get_int(query, 28);
        // gp
        client->coins.gp            = query_get_int(query, 29);
        // sp
        client->coins.sp            = query_get_int(query, 30);
        // cp
        client->coins.cp            = query_get_int(query, 31);
        // ppCursor
        client->coinsCursor.pp      = query_get_int(query, 32);
        // gpCursor
        client->coinsCursor.gp      = query_get_int(query, 33);
        // spCursor
        client->coinsCursor.sp      = query_get_int(query, 34);
        // cpCursor
        client->coinsCursor.cp      = query_get_int(query, 35);
        // ppBank
        client->coinsBank.pp        = query_get_int(query, 36);
        // gpBank
        client->coinsBank.gp        = query_get_int(query, 37);
        // spBank
        client->coinsBank.sp        = query_get_int(query, 38);
        // cpBank
        client->coinsBank.cp        = query_get_int(query, 39);
        // hunger
        client->hungerLevel         = query_get_int(query, 40);
        // thirst
        client->thirstLevel         = query_get_int(query, 41);
        // isGM
        client->isGM                = query_get_int(query, 42);
        // anon
        client->anonSetting         = query_get_int(query, 43);
        // drunkeness
        client->drunkeness          = query_get_int(query, 44);
        // creationTime
        client->creationTimestamp   = query_get_int64(query, 45);

        skills_init(&client->skills, mob_class(mob), mob_level(mob), mob_base_race(mob));
    }
    
    client->loaded.stats = true;
    
    // Ref to this client carries over into the following query
    
    // Skills
    db = core_db(C(zc));
    query_init(&q);
    query_set_userdata(&q, client);
    db_prepare_literal(db, &q, "SELECT skill_id, value FROM skill WHERE character_id = ?", client_load_skills_callback);
    query_bind_int64(&q, 1, client_character_id(client));
    db_schedule(db, &q);
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
        slot.isBag          = query_get_int(query, 5);
        slot.itemId         = query_get_int(query, 6);
        
        //fixme: create Item object with pointer from shared memory region...
        
        inventory_add_from_database(basic, inv, &slot);
    }
    
    client->loaded.inventory = true;
    
    if (client->loaded.connection)
        client_check_loading_finished(client);
    
    client_drop(client);
}

static void client_load_spellbook_callback(R(Query*) query)
{
    R(Client*) client   = query_userdata_type(query, Client);
    R(Spellbook*) book  = &client->spellbook;
    R(ZC*) zc           = client_zone_cluster(client);
    
    while (query_select(query))
    {
        uint32_t slotId     = query_get_int(query, 1);
        uint32_t spellId    = query_get_int(query, 2);
        
        spellbook_add_from_database(zc, book, slotId, spellId);
    }
    
    client->loaded.spellbook = true;
    
    if (client->loaded.connection)
        client_check_loading_finished(client);
    
    client_drop(client);
}

static void client_load_memmed_spells_callback(R(Query*) query)
{
    R(Client*) client   = query_userdata_type(query, Client);
    R(Spellbook*) book  = &client->spellbook;
    R(ZC*) zc           = client_zone_cluster(client);
    
    while (query_select(query))
    {
        uint32_t slotId             = query_get_int(query, 1);
        uint32_t spellId            = query_get_int(query, 2);
        uint64_t recastTimestamp    = query_get_int64(query, 3);
        
        spellbook_add_memorized_from_database(zc, book, slotId, spellId, recastTimestamp);
    }
    
    client->loaded.memmedSpells = true;
    
    if (client->loaded.connection)
        client_check_loading_finished(client);
    
    client_drop(client);
}

static void client_load_bind_points_callback(R(Query*) query)
{
    R(Client*) client = query_userdata_type(query, Client);
    
    while (query_select(query))
    {
        uint32_t bindId = query_get_int(query, 1);
        int zoneId      = query_get_int(query, 2);
        float x         = query_get_double(query, 3);
        float y         = query_get_double(query, 4);
        float z         = query_get_double(query, 5);
        float heading   = query_get_double(query, 6);
        
        client_set_bind_point(client, bindId, zoneId, x, y, z, heading);
    }
    
    client_fill_in_missing_bind_points(client);
    
    client->loaded.bindPoints = true;
    
    if (client->loaded.connection)
        client_check_loading_finished(client);
    
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
    skills_preinit(&client->skills);
    
    client->isLocal     = zoning->isLocal;
    client->characterId = zoning->characterId;
    client->accountName = string_create_from_cstr(B(zc), zoning->accountName, strlen(zoning->accountName));
    client->accountId   = zoning->accountId;
    client->ipAddress   = zoning->ipAddress;
    
    timer_init(&client->timerKeepAlive, zc_timer_pool(zc), EQP_CLIENT_KEEP_ALIVE_DELAY_MS, NULL, NULL, false);
    
    // Character stats
    client_grab(client);
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(db, &query,
        "SELECT "
            "surname, level, class, race, gender, face, deity, x, y, z, heading, current_hp, current_mana, current_endurance, experience, training_points, "
            "base_str, base_sta, base_dex, base_agi, base_int, base_wis, base_cha, guild_id, guild_rank, harmtouch_timestamp, discipline_timestamp, "
            "pp, gp, sp, cp, pp_cursor, gp_cursor, sp_cursor, cp_cursor, pp_bank, gp_bank, sp_bank, cp_bank, hunger, thirst, is_gm, anon, drunkeness, "
            "strftime('%s', creation_time) "
        "FROM character WHERE character_id = ?", client_load_stats_callback);
    query_bind_int64(&query, 1, zoning->characterId);
    db_schedule(db, &query);
    
    // Inventory
    client_grab(client);
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(db, &query, 
        "SELECT slot_id, aug_slot_id, stack_amount, charges, is_bag, item_id FROM inventory "
        "WHERE character_id = ? "
        "ORDER BY slot_id ASC", // Putting lower slots first will be a decent micro-optimization in the average case
        client_load_inventory_callback);
    query_bind_int64(&query, 1, zoning->characterId);
    db_schedule(db, &query);
    
    // Spellbook
    client_grab(client);
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(db, &query, 
        "SELECT slot_id, spell_id FROM spellbook WHERE character_id = ? ORDER BY slot_id ASC",
        client_load_spellbook_callback);
    query_bind_int64(&query, 1, zoning->characterId);
    db_schedule(db, &query);
    
    // Memorized spells
    client_grab(client);
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(db, &query, 
        "SELECT slot_id, spell_id, recast_timestamp_milliseconds FROM memmed_spells WHERE character_id = ?",
        client_load_memmed_spells_callback);
    query_bind_int64(&query, 1, zoning->characterId);
    db_schedule(db, &query);
    
    // Bind points
    client_grab(client);
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(db, &query, 
        "SELECT bind_id, zone_id, x, y, z, heading FROM bind_point WHERE character_id = ? ORDER BY bind_id ASC",
        client_load_bind_points_callback);
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
    
    timer_deinit(&client->timerKeepAlive);
    
    inventory_deinit(&client->inventory);
    mob_deinit(&client->mob);
    
stub:
    if (client_handler(client))
        protocol_handler_drop(client_handler(client));
    
    free(client);
}

void client_catch_up_with_loading_progress(R(Client*) client)
{
    // This is called when the client is matched with its UDP connection;
    // stuff has already been loading for this client in the background,
    // so check if we should send some packets.
    
    client->loaded.connection   = true;
    client->loaded.unused       = true;
    client_check_loading_finished(client);
}

void client_check_loading_finished(R(Client*) client)
{
    R(ZC*) zc;
    
    if (client->loaded.total != 0xff)
        return;
    
    zc = client_zone_cluster(client);
    
    //fixme: add lua event_determine_base_stats or some such here
    
    if (client->expansion == ExpansionId_Trilogy)
    {
        R(PacketTrilogy*) packet;
        
        client_trilogy_send_player_profile(client);
        client_trilogy_send_zone_entry(client);

        packet = client_trilogy_make_op_weather(zc, 0, 0); //fixme: get weather from zone
        client_trilogy_schedule_packet_individual(client, packet);
        
        // Start keepalive packet flow
        timer_set_callback(&client->timerKeepAlive, client_trilogy_send_keep_alive);
        timer_set_userdata(&client->timerKeepAlive, client_handler(client));
        timer_start(&client->timerKeepAlive);
    }
    else
    {
        
    }
}

void client_fill_in_missing_bind_points(R(Client*) client)
{
    BindPoint* copyFrom = NULL;
    BindPoint qeynos;
    uint32_t i;
    
    for (i = 0; i < 5; i++)
    {
        if (client->bindPoints[i].zoneId)
        {
            copyFrom = &client->bindPoints[i];
            goto found;
        }
    }
    
    //fixme: find the proper loc for an all-else-fails bind point
    qeynos.zoneId       = 1;
    qeynos.loc.x        = 0.0f;
    qeynos.loc.y        = 0.0f;
    qeynos.loc.z        = 0.0f;
    qeynos.loc.heading  = 0.0f;
    copyFrom = &qeynos;
    
found:
    
    for (i = 0; i < 5; i++)
    {
        if (client->bindPoints[i].zoneId == 0)
            memcpy(&client->bindPoints[i], copyFrom, sizeof(BindPoint));
    }
}

int client_is_pvp(R(Client*) client)
{
    return client->isCurrentlyPvP;
}

int client_is_gm(R(Client*) client)
{
    return client->isGM;
}

int client_is_afk(R(Client*) client)
{
    return client->isAfk;
}

int client_is_linkdead(R(Client*) client)
{
    return client->isLinkdead;
}

uint8_t client_anon_setting(R(Client*) client)
{
    return client->anonSetting;
}

uint8_t client_guild_rank(R(Client*) client)
{
    return client->guildRank;
}

const char* client_surname_cstr(R(Client*) client)
{
    return (client->surname) ? string_data(client->surname) : "";
}

void client_set_bind_point(R(Client*) client, uint32_t bindId, int zoneId, float x, float y, float z, float heading)
{
    R(BindPoint*) bind;
    
    if (bindId >= 5)
        return;
    
    bind = &client->bindPoints[bindId];
    
    bind->zoneId        = zoneId;
    bind->loc.x         = x;
    bind->loc.y         = y;
    bind->loc.z         = z;
    bind->loc.heading   = heading;
}
