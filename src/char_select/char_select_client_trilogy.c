
#include "char_select_client_trilogy.h"
#include "eqp_char_select.h"

#define WEARCHANGE_OP_SWITCH_CHAR   0x7fff

void* client_create_from_new_connection_trilogy(ProtocolHandler* handler)
{
    return char_select_client_create(handler, ExpansionId_Trilogy);
}

static void cs_trilogy_schedule_packet(ProtocolHandler* handler, PacketTrilogy* packet)
{
    packet_trilogy_fragmentize(packet);
    protocol_handler_trilogy_schedule_packet(&handler->trilogy, packet);
}

static void cs_trilogy_echo_zero_length(ProtocolHandler* handler, uint16_t opcode)
{
    PacketTrilogy* packet = packet_trilogy_create(protocol_handler_basic(handler), opcode, 0);
    cs_trilogy_schedule_packet(handler, packet);
}

static void cs_trilogy_handle_op_login_info(CharSelectClient* client, ProtocolHandler* handler, Aligned* a)
{
    Basic* basic;
    const char* account;
    const char* sessionKey;
    
    if (aligned_remaining(a) == 0)
        return;
    
    account = (const char*)aligned_current(a); // "LS#" + digits
    aligned_advance_null_terminator(a);
    
    if (aligned_remaining(a) == 0 || strlen(account) < 4)
        return;
    
    sessionKey  = (const char*)aligned_current(a);
    basic       = protocol_handler_basic(handler);
    
    client->auth.accountId = strtol(account + 3, NULL, 10); // Skip "LS#"
    snprintf(client->auth.sessionKey, sizeof_field(CharSelectAuth, sessionKey), "%s", sessionKey);
    
    char_select_handle_unauthed_client((CharSelect*)protocol_handler_basic(handler), client);
}

void cs_client_trilogy_on_auth(CharSelectClient* client)
{
    ProtocolHandler* handler    = char_select_client_handler(client);
    Basic* basic                = protocol_handler_basic(handler);
    Aligned w;
    PacketTrilogy* packet;
    
    packet = packet_trilogy_create(basic, TrilogyOp_LoginApproved, 1);
    packet_trilogy_data(packet)[0] = 0;
    cs_trilogy_schedule_packet(handler, packet);
    
    packet = packet_trilogy_create(basic, TrilogyOp_Enter, 1);
    packet_trilogy_data(packet)[0] = 0;
    cs_trilogy_schedule_packet(handler, packet);
    
    packet = packet_trilogy_create(basic, TrilogyOp_ExpansionInfo, sizeof(uint32_t));
    aligned_init(basic, &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    aligned_write_uint32(&w, 1 | 2); // Bitfield: 1 = Kunark, 2 = Velious, 4 = Luclin
    cs_trilogy_schedule_packet(handler, packet);
    
    char_select_client_query_account_id(client, (CharSelect*)basic);
}

static void cs_client_trilogy_characters_callback(Query* query)
{
    CharSelectClient* client    = query_userdata_type(query, CharSelectClient);
    ProtocolHandler* handler    = char_select_client_handler(client);
    PacketTrilogy* packet;
    CSTrilogy_CharSelectInfo* cs;
    uint32_t i;
    
    /*
        This packet is about the worst case scenario for using the Aligned writer...
        would need to jump around a lot, backwards and forwards. So instead of that, we'll just
        do all writes in terms of memcpy, snprintf or single bytes to ensure they are aligned.
    */
    
    packet  = packet_trilogy_create(protocol_handler_basic(handler), TrilogyOp_CharacterInfo, sizeof(CSTrilogy_CharSelectInfo));
    cs      = (CSTrilogy_CharSelectInfo*)packet_trilogy_data(packet);
    
    memset(cs, 0, sizeof(CSTrilogy_CharSelectInfo));
    
    // We need to set these (probably not exactly like this, but close enough) to be able to
    // tell which character slot is being referred to when the client asks for which weapon
    // models to display in characters' hands, via WearChange requests.
    for (i = 0; i < 10; i++)
    {
        memset(cs->weirdA[i], i, sizeof(uint32_t));
        memset(cs->weirdB[i], i, sizeof(uint32_t));
    }
    
    i = 0;
    
    while (query_select(query))
    {
        uint32_t val32;
        int16_t val16;
        uint8_t* materials;
        uint32_t* tints;
        
        snprintf(cs->names[i], 30, "%s", query_get_string(query, 1, NULL));
        
        cs->levels[i]   = (uint8_t)query_get_int(query, 2);
        cs->classes[i]  = (uint8_t)query_get_int(query, 3);
        
        val16 = (int16_t)query_get_int(query, 4);
        memcpy(&cs->races[i], &val16, sizeof(int16_t));
        
        snprintf(cs->zoneShortNames[i], 20, "%s", zone_short_name_by_id(query_get_int(query, 5)));
        
        cs->genders[i]  = (uint8_t)query_get_int(query, 6);
        cs->faces[i]    = (uint8_t)query_get_int(query, 7);
        
        // Unrolled loops
        materials = cs->materials[i];
        
        materials[0] = (uint8_t)query_get_int(query, 8);
        materials[1] = (uint8_t)query_get_int(query, 9);
        materials[2] = (uint8_t)query_get_int(query, 10);
        materials[3] = (uint8_t)query_get_int(query, 11);
        materials[4] = (uint8_t)query_get_int(query, 12);
        materials[5] = (uint8_t)query_get_int(query, 13);
        materials[6] = (uint8_t)query_get_int(query, 14);
        
        // Weapon materials are handled specially and separately by the client 
        // for no apparent reason

        client->weaponMaterialsTrilogy[i][0] = (uint8_t)query_get_int64(query, 15);
        client->weaponMaterialsTrilogy[i][1] = (uint8_t)query_get_int64(query, 16);
        
        tints = cs->materialTints[i];
        
        val32 = (uint32_t)query_get_int64(query, 17);
        memcpy(&tints[0], &val32, sizeof(uint32_t));
        
        val32 = (uint32_t)query_get_int64(query, 18);
        memcpy(&tints[1], &val32, sizeof(uint32_t));
        
        val32 = (uint32_t)query_get_int64(query, 19);
        memcpy(&tints[2], &val32, sizeof(uint32_t));
        
        val32 = (uint32_t)query_get_int64(query, 20);
        memcpy(&tints[3], &val32, sizeof(uint32_t));
        
        val32 = (uint32_t)query_get_int64(query, 21);
        memcpy(&tints[4], &val32, sizeof(uint32_t));
        
        val32 = (uint32_t)query_get_int64(query, 22);
        memcpy(&tints[5], &val32, sizeof(uint32_t));
        
        val32 = (uint32_t)query_get_int64(query, 23);
        memcpy(&tints[6], &val32, sizeof(uint32_t));
        
        i++;
    }
    
    // Handle empty spaces
    for (; i < 10; i++)
    {
        snprintf(cs->names[i], 30, "<none>");
        snprintf(cs->zoneShortNames[i], 20, "qeynos");
    }
    
    cs_trilogy_schedule_packet(handler, packet);
    char_select_client_drop(client);
}

void cs_client_trilogy_on_account_id(CharSelectClient* client, uint32_t accountId)
{
    Database* db = core_db(C(protocol_handler_basic(char_select_client_handler(client))));
    Query query;
    
    char_select_client_grab(client);
    
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(db, &query,
        "SELECT "
            "name, level, class, race, zone_id, gender, face, "
            "material0, material1, material2, material3, material4, material5, material6, material7, material8, "
            "tint0, tint1, tint2, tint3, tint4, tint5, tint6 "
        "FROM character "
        "WHERE name_id_pair = ? "
        "ORDER BY character_id ASC LIMIT 10",
        cs_client_trilogy_characters_callback);
    
    query_bind_int64(&query, 1, (int64_t)accountId);
    
    db_schedule(db, &query);
}

static void cs_trilogy_handle_op_guild_list(CharSelectClient* client, ProtocolHandler* handler)
{
    Aligned write;
    Aligned* w  = &write;
    PacketTrilogy* packet;
    uint32_t i;
    
    if (!char_select_client_is_authed(client))
        return;
    
    // The entire guild list struct is properly aligned on a 4-byte boundary
    packet = packet_trilogy_create(protocol_handler_basic(handler), TrilogyOp_GuildList, sizeof(CSTrilogy_GuildList));
    
    aligned_init(protocol_handler_basic(handler), w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    aligned_write_zero_all(w);
    
    // unknown
    aligned_advance(w, sizeof(uint32_t));
    
    for (i = 0; i < EQP_CHAR_SELECT_MAX_GUILDS; i++)
    {
        // guildId
        aligned_write_uint32(w, 0xffffffff);
        // name
        aligned_advance(w, 32);
        // unknownA
        aligned_write_uint32(w, 0xffffffff);
        // exists, unknownB
        aligned_advance(w, sizeof(uint32_t) * 2);
        // unknownC
        aligned_write_uint32(w, 0xffffffff);
        // unknownD
        aligned_advance(w, sizeof(uint32_t));
    }
    
    cs_trilogy_schedule_packet(handler, packet);
}

static void cs_trilogy_handle_op_name_approval(CharSelectClient* client, ProtocolHandler* handler, Aligned* a)
{
    const char* name;
    uint32_t race;
    uint32_t class;
    
    if (!char_select_client_is_authed(client) || aligned_remaining(a) < sizeof(CSTrilogy_NameApproval))
        return;
    
    name    = (const char*)aligned_current(a);
    aligned_advance(a, sizeof_field(CSTrilogy_NameApproval, name));
    race    = aligned_read_uint32(a);
    class   = aligned_read_uint32(a);
    
    char_select_client_query_character_name_taken(client, (CharSelect*)protocol_handler_basic(handler), name);
}

void cs_client_trilogy_on_character_name_checked(CharSelectClient* client, int taken)
{
    ProtocolHandler* handler    = char_select_client_handler(client);
    PacketTrilogy* packet       = packet_trilogy_create(protocol_handler_basic(handler), TrilogyOp_NameApproval, 1);
    int available               = !taken;
    
    if (available)
        char_select_client_set_name_approved(client, true);
    
    packet_trilogy_data(packet)[0] = (uint8_t)available;
    cs_trilogy_schedule_packet(handler, packet);
}

static int cs_client_trilogy_char_creation_params_are_valid(CharCreateLua* ccl)
{
    uint32_t class  = char_create_lua_class(ccl) - 1;
    uint32_t race   = char_create_lua_race(ccl) - 1;
    uint32_t totalExpected;
    uint32_t total;
    uint32_t maxSpendableOneStat;
    int i;
    
    /*
        classesByRace encodes the following matrix, using 1 bit per entry:
        
                       Human  Barbarian Erudite Woodelf Highelf Darkelf Halfelf Dwarf  Troll  Ogre   Halfling Gnome  Iksar
        Warrior      { true,  true,     false,  true,   false,  true,   true,   true,  true,  true,  true,    true,	 true  },
        Cleric       { true,  false,    true,   false,  true,   true,   false,  true,  false, false, true,    true,	 false },
        Paladin      { true,  false,    true,   false,  true,   false,  true,   true,  false, false, true,    true,	 false },
        Ranger       { true,  false,    false,  true,   false,  false,  true,   false, false, false, true,    false, false },
        ShadowKnight { true,  false,    true,   false,  false,  true,   false,  false, true,  true,  false,   true,  true  },
        Druid        { true,  false,    false,  true,   false,  false,  true,   false, false, false, true,    false, false },
        Monk         { true,  false,    false,  false,  false,  false,  false,  false, false, false, false,   false, true  },
        Bard         { true,  false,    false,  true,   false,  false,  true,   false, false, false, false,   false, false },
        Rogue        { true,  true,     false,  true,   false,  true,   true,   true,  false, false, true,    true,  false },
        Shaman       { false, true,     false,  false,  false,  false,  false,  false, true,  true,  false,   false, true  },
        Necromancer  { true,  false,    true,   false,  false,  true,   false,  false, false, false, false,   true,  true  },
        Wizard       { true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false },
        Magician     { true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false },
        Enchanter    { true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false }
    */
    static const uint16_t classesByRace[13] = {
        0x3dff, // Human
        0x0301, // Barbarian
        0x3c16, // Erudite
        0x01a9, // Wood Elf
        0x3806, // High Elf
        0x3d13, // Dark Elf
        0x01ad, // Half Elf
        0x0107, // Drawf
        0x0211, // Troll
        0x0211, // Ogre
        0x012f, // Halfling
        0x3d17, // Gnome
        0x0651  // Iksar
    };
    
    static const uint8_t baseStatsByRace[13][7] = {
        /*                 STR  STA  CHA  DEX  INT  AGI  WIS */
        /* Human     */ {  75,  75,  75,  75,  75,  75,  75 },
        /* Barbarian */ { 103,  95,  55,  70,  60,  82,  70 },
        /* Erudite   */ {  60,  70,  70,  70, 107,  70,  83 },
        /* Wood Elf  */ {  65,  65,  75,  80,  75,  95,  80 },
        /* High Elf  */ {  55,  65,  80,  70,  92,  85,  95 },
        /* Dark Elf  */ {  60,  65,  60,  75,  99,  90,  83 },
        /* Half Elf  */ {  70,  70,  75,  85,  75,  90,  60 },
        /* Dwarf     */ {  90,  90,  45,  90,  60,  70,  83 },
        /* Troll     */ { 108, 109,  40,  75,  52,  83,  60 },
        /* Ogre      */ { 130, 122,  37,  70,  60,  70,  67 },
        /* Halfling  */ {  70,  75,  50,  90,  67,  95,  80 },
        /* Gnome     */ {  60,  70,  60,  85,  98,  85,  67 },
        /* Iksar     */ {  70,  70,  55,  85,  75,  90,  80 }
    };
    
    static const uint8_t statBonusesByClass[14][7] = {
        /*                   STR STA CHA DEX INT AGI WIS */
        /* Warrior      */ { 10, 10,  0,  0,  0,  5,  0 },
        /* Cleric       */ {  5,  5,  0,  0,  0,  0, 10 },
        /* Paladin      */ { 10,  5, 10,  0,  0,  0,  5 },
        /* Ranger       */ {  5, 10,  0,  0,  0, 10,  5 },
        /* ShadowKnight */ { 10,  5,  5,  0, 10,  0,  0 },
        /* Druid        */ {  0, 10,  0,  0,  0,  0, 10 },
        /* Monk         */ {  5,  5,  0, 10,  0, 10,  0 },
        /* Bard         */ {  5,  0, 10, 10,  0,  0,  0 },
        /* Rogue        */ {  0,  0,  0, 10,  0, 10,  0 },
        /* Shaman       */ {  0,  5,  5,  0,  0,  0, 10 },
        /* Necromancer  */ {  0,  0,  0, 10, 10,  0,  0 },
        /* Wizard       */ {  0, 10,  0,  0, 10,  0,  0 },
        /* Magician     */ {  0, 10,  0,  0, 10,  0,  0 },
        /* Enchanter    */ {  0,  0, 10,  0, 10,  0,  0 }
    };
    
    static const uint8_t spendableStatPointsByClass[14] = {
        25, // Warrior
        30, // Cleric
        20, // Paladin
        20, // Ranger
        20, // ShadowKnight
        30, // Druid
        20, // Monk
        25, // Bard
        30, // Rogue
        30, // Shaman
        30, // Necromancer
        30, // Wizard
        30, // Magician
        30  // Enchanter
    };
    
    // Iksar index correction
    if (race > 12)
        race = 12;

    // Is the race/class combination valid?
    if (class >= 14 || (classesByRace[race] & (1 << class)) == 0)
        return false;
    
    totalExpected       = spendableStatPointsByClass[class];
    total               = 0;
    maxSpendableOneStat = totalExpected;
    
    if (maxSpendableOneStat > 25)
        maxSpendableOneStat = 25;
    
    for (i = 0; i < 7; i++)
    {
        uint32_t statValue  = ccl->stats[i];
        uint32_t statTotal  = baseStatsByRace[race][i] + statBonusesByClass[class][i];

        // Does this stat exceed the maximum possible value for this race and class?
        if (statValue > (statTotal + maxSpendableOneStat))
            return false;
        
        totalExpected += statTotal;
        total += statValue;
    }
    
    // Do they have more stat points overall than should be possible?
    if (total > totalExpected)
        return false;
    
    return true;
}

static void cs_client_trilogy_send_char_create_failure(CharSelectClient* client)
{
    ProtocolHandler* handler    = char_select_client_handler(client);
    PacketTrilogy* packet       = packet_trilogy_create(protocol_handler_basic(handler), TrilogyOp_NameApproval, 1);
    
    char_select_client_set_name_approved(client, false);
    
    packet_trilogy_data(packet)[0] = 0;
    cs_trilogy_schedule_packet(handler, packet);
}

static void cs_client_trilogy_transaction(Transaction* trans)
{
    CharCreateLua* ccl      = transaction_userdata_type(trans, CharCreateLua);
    int64_t charId          = char_create_lua_character_id(ccl);
    CharCreateItem* items   = array_data_type(ccl->startingItems, CharCreateItem);
    uint32_t n              = array_count(ccl->startingItems);
    Database* db            = transaction_db(trans);
    Query query;
    uint32_t i;
    
    // Bind points
    
    query_init(&query);
    db_prepare_literal(db, &query, "INSERT INTO bind_point "
        "(character_id, bind_id, zone_id, x, y, z, heading) "
        "VALUES "
        "(?, ?, ?, ?, ?, ?, ?)", NULL);
    
    query_bind_int64(&query, 1, charId);
    
    for (i = 0; i < 5; i++)
    {
        CharCreatePoint* b = &ccl->bindPoints[i];
        
        if (b->zoneId == 0)
            continue;
        
        query_bind_int(&query, 2, i);
        query_bind_int(&query, 3, b->zoneId);
        query_bind_double(&query, 4, b->x);
        query_bind_double(&query, 5, b->y);
        query_bind_double(&query, 6, b->z);
        query_bind_double(&query, 7, b->heading);
        
        query_execute_synchronus_insert_update(&query);
    }
    
    query_deinit(&query);
    
    // Starting inventory
    
    query_init(&query);
    db_prepare_literal(db, &query, "INSERT INTO inventory "
        "(character_id, slot_id, stack_amount, charges, item_id) "
        "VALUES "
        "(?, ?, ?, ?, ?)", NULL);
    
    query_bind_int64(&query, 1, charId);
    
    for (i = 0; i < n; i++)
    {
        CharCreateItem* item = &items[i];
        
        query_bind_int(&query, 2, item->slotId);
        query_bind_int(&query, 3, item->stackAmount);
        query_bind_int(&query, 4, item->charges);
        query_bind_int(&query, 5, item->itemId);
        
        query_execute_synchronus_insert_update(&query);
    }
    
    query_deinit(&query);
}

static void cs_client_trilogy_transaction_finished(Query* query)
{
    CharCreateLua* ccl          = query_userdata_type(query, CharCreateLua);
    CharSelectClient* client    = char_create_lua_client(ccl);
    
    cs_client_trilogy_on_account_id(client, char_select_client_account_id(client));
    
    char_select_client_drop(client);
    char_create_lua_destroy(ccl);
}

static void cs_client_trilogy_create_character_callback(Query* query)
{
    CharCreateLua* ccl          = query_userdata_type(query, CharCreateLua);
    CharSelectClient* client    = char_create_lua_client(ccl);
    
    if (query_affected_rows(query) == 0)
    {
        cs_client_trilogy_send_char_create_failure(client);
    }
    else if (char_create_lua_has_bind_points_or_items(ccl))
    {
        Database* db = query_get_db(query);
        char_create_lua_set_character_id(ccl, query_last_insert_id(query));
        db_schedule_transaction(db, ccl, cs_client_trilogy_transaction, cs_client_trilogy_transaction_finished);
        return;
    }
    else
    {
        cs_client_trilogy_on_account_id(client, char_select_client_account_id(client));
    }
    
    char_select_client_drop(client);
    char_create_lua_destroy(ccl);
}

static void cs_trilogy_handle_op_create_character(CharSelectClient* client, ProtocolHandler* handler, Aligned* a)
{
    CharCreateLua* ccl;
    CharSelect* charSelect;
    CharCreatePoint p;
    CharCreatePoint bind[5];
    Database* db;
    Query query;
    uint32_t i;
    
    if (!char_select_client_is_authed(client) || !char_select_client_is_name_approved(client) || aligned_remaining(a) < sizeof(Trilogy_PlayerProfile_CharCreation))
        return;
    
    charSelect  = (CharSelect*)protocol_handler_basic(handler);
    ccl         = char_create_lua_create(charSelect, client, true);
    
    char_create_lua_set_account_id(ccl, char_select_client_account_id(client));
    
    // name
    char_create_lua_set_name(ccl, (const char*)aligned_current(a));
    aligned_advance(a, sizeof_field(Trilogy_PlayerProfile_CharCreation, name) + sizeof_field(Trilogy_PlayerProfile_CharCreation, surname));
    // gender
    char_create_lua_set_gender(ccl, aligned_read_uint16(a));
    // race
    char_create_lua_set_race(ccl, aligned_read_uint16(a));
    // class
    char_create_lua_set_class(ccl, aligned_read_uint16(a));
    // level, experience, trainingPoints, currentMana
    aligned_advance(a, 
        sizeof_field(Trilogy_PlayerProfile_CharCreation, level)             +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, experience)        +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, trainingPoints)    +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, currentMana));
    // face
    char_create_lua_set_face(ccl, aligned_read_uint8(a));
    // unknownA[47]
    aligned_advance(a, sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownA));
    // currentHp
    char_create_lua_set_current_hp(ccl, aligned_read_int16(a));
    // unknownB
    aligned_advance(a, sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownB));
    // STR
    char_create_lua_set_str(ccl, aligned_read_uint8(a));
    // STA
    char_create_lua_set_sta(ccl, aligned_read_uint8(a));
    // CHA
    char_create_lua_set_cha(ccl, aligned_read_uint8(a));
    // DEX
    char_create_lua_set_dex(ccl, aligned_read_uint8(a));
    // INT
    char_create_lua_set_int(ccl, aligned_read_uint8(a));
    // AGI
    char_create_lua_set_agi(ccl, aligned_read_uint8(a));
    // WIS
    char_create_lua_set_wis(ccl, aligned_read_uint8(a));
    // languages, unknownC, mainInventory, buffs, baggedItems, spellbook, memmedSpellIds, unknownD
    aligned_advance(a,
        sizeof_field(Trilogy_PlayerProfile_CharCreation, languages)                     +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownC)                      +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, mainInventoryItemIds)          +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, mainInventoryInternalUnused)   +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, mainInventoryItemProperties)   +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, buffs)                         +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, baggedItemIds)                 +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, baggedItemProperties)          +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, spellbook)                     +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, memmedSpellIds)                +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownD));
    // y
    p.y         = aligned_read_float(a);
    // x
    p.x         = aligned_read_float(a);
    // z
    p.z         = aligned_read_float(a);
    // heading
    p.heading   = aligned_read_float(a);
    // zoneShortName
    p.zoneId    = char_select_get_zone_id(charSelect, (const char*)aligned_current(a));
    aligned_advance(a, sizeof_field(Trilogy_PlayerProfile_CharCreation, zoneShortName));
    
    char_create_lua_set_starting_zone(ccl, p.zoneId, p.x, p.y, p.z, p.heading);
    
    // lots of stuff...
    aligned_advance(a,
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownEDefault100)    +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, coins)                 +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, coinsBank)             +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, coinsCursor)           +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, skills)                +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownF)              +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, autoSplit)             +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, pvpEnabled)            +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownG)              +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, isGM)                  +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownH)              +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, disciplinesReady)      +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownI)              +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, hunger)                +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, thirst)                +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownJ));
        
    // bindZoneShortName[5]
    for (i = 0; i < 5; i++)
    {
        bind[i].zoneId = char_select_get_zone_id(charSelect, (const char*)aligned_current(a));
        aligned_advance(a, sizeof_field(Trilogy_PlayerProfile_CharCreation, bindZoneShortName[0]));
    }
    
    // bankInventoryItemProperties, bankedBaggedItemProperties, unknownK
    aligned_advance(a,
        sizeof_field(Trilogy_PlayerProfile_CharCreation, bankInventoryItemProperties)   +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, bankBaggedItemProperties)      +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownK)                      +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownL));
    
    // bindLocY[5]
    for (i = 0; i < 5; i++)
    {
        bind[i].y = aligned_read_float(a);
    }
    
    // bindLocX[5]
    for (i = 0; i < 5; i++)
    {
        bind[i].x = aligned_read_float(a);
    }
    
    // bindLocZ[5]
    for (i = 0; i < 5; i++)
    {
        bind[i].z = aligned_read_float(a);
    }
    
    // bindLocHeading[5]
    /*
    for (i = 0; i < 5; i++)
    {
        bind[i].heading = aligned_read_float(a);
    }
    */
    
    for (i = 0; i < 5; i++)
    {
        CharCreatePoint* b = &bind[i];
        char_create_lua_set_bind_point(ccl, b->zoneId, b->x, b->y, b->z, 0, i);
    }
    
    // more stuff
    aligned_advance(a,
        sizeof_field(Trilogy_PlayerProfile_CharCreation, bankInventoryInternalUnused)   +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownM)                      +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unixTimeA)                     +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownN)                      +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownODefault1)              +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, unknownP)                      +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, bankInventoryItemIds)          +
        sizeof_field(Trilogy_PlayerProfile_CharCreation, bankBaggedItemIds));
    
    // deity
    char_create_lua_set_deity(ccl, aligned_read_uint16(a));
    
    // Verify that the given character creation parameters are valid
    if (!cs_client_trilogy_char_creation_params_are_valid(ccl))
    {
        char_create_lua_destroy(ccl);
        cs_client_trilogy_send_char_create_failure(client);
        return;
    }
    
    char_select_char_create_lua_event(charSelect, ccl);

    char_select_client_grab(client);
    
    db = core_db(C(charSelect));
    
    query_init(&query);
    query_set_userdata(&query, ccl);
    db_prepare_literal(db, &query,
        "INSERT OR IGNORE INTO character " // Why OR IGNORE? Just in case two people try to make a character with the same name at the same time...
            "(name_id_pair, name, gender, race, class, face, current_hp, base_str, base_sta, base_cha, base_dex, base_int, base_agi, base_wis, deity, zone_id, x, y, z, heading) "
        "VALUES "
            "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        cs_client_trilogy_create_character_callback);
    
    // accountId
    query_bind_int64(&query, 1, char_create_lua_account_id(ccl));
    // name
    query_bind_string(&query, 2, char_create_lua_name(ccl), QUERY_CALC_LENGTH);
    // gender
    query_bind_int(&query, 3, char_create_lua_gender(ccl));
    // race
    query_bind_int(&query, 4, char_create_lua_race(ccl));
    // class
    query_bind_int(&query, 5, char_create_lua_class(ccl));
    // face
    query_bind_int(&query, 6, char_create_lua_face(ccl));
    // currentHp
    query_bind_int(&query, 7, char_create_lua_current_hp(ccl));
    // STR
    query_bind_int(&query, 8, char_create_lua_str(ccl));
    // STA
    query_bind_int(&query, 9, char_create_lua_sta(ccl));
    // CHA
    query_bind_int(&query, 10, char_create_lua_cha(ccl));
    // DEX
    query_bind_int(&query, 11, char_create_lua_dex(ccl));
    // INT
    query_bind_int(&query, 12, char_create_lua_int(ccl));
    // AGI
    query_bind_int(&query, 13, char_create_lua_agi(ccl));
    // WIS
    query_bind_int(&query, 14, char_create_lua_wis(ccl));
    // deity
    query_bind_int(&query, 15, char_create_lua_deity(ccl));
    // zoneId
    query_bind_int(&query, 16, char_create_lua_zone_id(ccl));
    // x
    query_bind_double(&query, 17, char_create_lua_zone_x(ccl));
    // y
    query_bind_double(&query, 18, char_create_lua_zone_y(ccl));
    // z
    query_bind_double(&query, 19, char_create_lua_zone_z(ccl));
    // heading
    query_bind_double(&query, 20, char_create_lua_zone_heading(ccl));
    
    db_schedule(db, &query);
}

static void cs_trilogy_handle_op_delete_character(CharSelectClient* client, ProtocolHandler* handler, Aligned* a)
{
    const char* name;
    
    if (!char_select_client_is_authed(client) || aligned_remaining(a) < 2)
        return;
    
    name = (const char*)aligned_current(a);
    
    char_select_client_delete_character_by_name(client, (CharSelect*)protocol_handler_basic(handler), name);
}

static void cs_trilogy_handle_op_wear_change(CharSelectClient* client, ProtocolHandler* handler, Aligned* a)
{
    uint8_t slot;
    uint16_t op;
    uint8_t flag;
    uint8_t index;
    
    if (!char_select_client_is_authed(client) || aligned_remaining(a) < sizeof(CSTrilogy_WearChange))
        return;
    
    // unusedSpawnId
    aligned_advance(a, sizeof(uint32_t));
    // slot
    slot    = aligned_read_uint8(a);
    // material
    index   = aligned_read_uint8(a);
    // operation
    op      = aligned_read_uint16(a);
    // color, unknownA
    aligned_advance(a, sizeof(uint32_t) + sizeof(uint8_t));
    // flag
    flag    = aligned_read_uint8(a);
	
    // If the flag is 0xb1, it is some kind of echo -- don't reply or it'll cause endless spam
    if (op != WEARCHANGE_OP_SWITCH_CHAR && flag != 0xb1)
    {
        Basic* basic;
        PacketTrilogy* packet;
        Aligned w;

        if (index >= 10 || (slot != 7 && slot != 8))
            return;
        
        basic   = protocol_handler_basic(handler);
        packet  = packet_trilogy_create(basic, TrilogyOp_WearChange, sizeof(CSTrilogy_WearChange));
        
        aligned_init(basic, &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
        
        // unusedSpawnId
        aligned_write_uint32(&w, 0);
        // slot
        aligned_write_uint8(&w, slot);
        // material
        aligned_write_uint8(&w, char_select_client_weapon_material(client, index, slot));
        // operation, color, unknownA, flag, unknownB
        aligned_write_zeroes(&w, sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint8_t) * 2 + sizeof(uint16_t));
        
        cs_trilogy_schedule_packet(handler, packet);
    }
}

static void cs_trilogy_handle_op_enter(CharSelectClient* client, ProtocolHandler* handler, Aligned* a)
{
    if (!char_select_client_is_authed(client) || aligned_remaining(a) < 2)
        return;
    
    char_select_send_client_zone_in_request((CharSelect*)protocol_handler_basic(handler), client, handler, (const char*)aligned_current(a));
}

void cs_client_trilogy_on_zone_in_failure(CharSelectClient* client, CharSelect* charSelect, const char* zoneShortName)
{
    PacketTrilogy* packet       = packet_trilogy_create(B(charSelect), TrilogyOp_ZoneUnavailable, sizeof(CSTrilogy_ZoneUnavailable));
    ProtocolHandler* handler    = char_select_client_handler(client);
    Aligned w;
    
    aligned_init(B(charSelect), &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    aligned_write_snprintf_full_advance(&w, sizeof_field(CSTrilogy_ZoneUnavailable, shortName), "%s", zoneShortName);
    
    cs_trilogy_schedule_packet(handler, packet);
}

void cs_client_trilogy_on_zone_in_success(CharSelectClient* client, CharSelect* charSelect, Server_ZoneAddress* zoneAddr)
{
    ProtocolHandler* handler = char_select_client_handler(client);
    PacketTrilogy* packet;
    EQ_Time eqTime;
    Aligned write;
    Aligned* w  = &write;
    uint32_t length;
    
    aligned_set_basic(w, B(charSelect));
    
    // MotD
    length  = zoneAddr->motdLength;
    packet  = packet_trilogy_create(B(charSelect), TrilogyOp_MessageOfTheDay, length + 1);
    aligned_reinit(w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    aligned_write_buffer(w, zoneAddr->messageOfTheDay, length);
    aligned_write_byte(w, 0);
    
    cs_trilogy_schedule_packet(handler, packet);
    
    // TimeOfDay
    packet = packet_trilogy_create(B(charSelect), TrilogyOp_TimeOfDay, sizeof(CSTrilogy_TimeOfDay));
    aligned_reinit(w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    // This is the game world time, not real time
    eq_time_calc(&eqTime, zoneAddr->eqTimeBaseUnixSeconds);
    
    // hour
    aligned_write_uint8(w, eqTime.hour);
    // minute
    aligned_write_uint8(w, eqTime.minute);
    // day
    aligned_write_uint8(w, eqTime.day);
    // month
    aligned_write_uint8(w, eqTime.month);
    // year
    aligned_write_uint16(w, eqTime.year);
    
    cs_trilogy_schedule_packet(handler, packet);
    
    // Zone address
    packet = packet_trilogy_create(B(charSelect), TrilogyOp_ZoneAddress, sizeof(CSTrilogy_ZoneAddress));
    aligned_reinit(w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    // ipAddress
    aligned_write_snprintf_full_advance(w, sizeof_field(CSTrilogy_ZoneAddress, ipAddress), "%s", zoneAddr->ipAddress);
        
    // zoneShortName
    aligned_write_snprintf_full_advance(w, sizeof_field(CSTrilogy_ZoneAddress, zoneShortName), "%s", zoneAddr->shortName);
    // port
    aligned_write_uint16(w, toNetworkUint16(zoneAddr->portHostByteOrder));
    
    cs_trilogy_schedule_packet(handler, packet);
}

void client_recv_packet_trilogy(void* vclient, uint16_t opcode, Aligned* a)
{
    CharSelectClient* client    = (CharSelectClient*)vclient;
    ProtocolHandler* handler    = char_select_client_handler(client);

    log_format(protocol_handler_basic(handler), LogNetwork, "Received packet opcode 0x%04x, length %u", opcode, aligned_remaining(a));
    
    switch (opcode)
    {
    case TrilogyOp_LoginInfo:
        cs_trilogy_handle_op_login_info(client, handler, a);
        break;
    
    case TrilogyOp_GuildList:
        cs_trilogy_handle_op_guild_list(client, handler);
        break;
    
    case TrilogyOp_NameApproval:
        cs_trilogy_handle_op_name_approval(client, handler, a);
        break;
    
    case TrilogyOp_CreateCharacter:
        cs_trilogy_handle_op_create_character(client, handler, a);
        break;
    
    case TrilogyOp_DeleteCharacter:
        cs_trilogy_handle_op_delete_character(client, handler, a);
        break;
    
    case TrilogyOp_WearChange:
        cs_trilogy_handle_op_wear_change(client, handler, a);
        break;
    
    case TrilogyOp_Enter:
        cs_trilogy_handle_op_enter(client, handler, a);
        break;
    
    // Special, zero-length packets that need to be echoed for whatever reason
    case 0x2023:
    case 0x80a9:
    case 0xab00:
    case 0xac00:
    case 0xad00:
        cs_trilogy_echo_zero_length(handler, opcode);
        break;
    
    default:
        break;
    }
}

#undef WEARCHANGE_OP_SWITCH_CHAR
