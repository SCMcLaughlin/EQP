
#include "char_select_client_trilogy.h"
#include "eqp_char_select.h"

void* client_create_from_new_connection_trilogy(R(ProtocolHandler*) handler)
{
    return char_select_client_create(handler, ExpansionId_Trilogy);
}

static void cs_trilogy_schedule_packet(R(ProtocolHandler*) handler, R(PacketTrilogy*) packet)
{
    packet_trilogy_fragmentize(packet);
    protocol_handler_trilogy_schedule_packet(&handler->trilogy, packet);
}

static void cs_trilogy_echo_zero_length(R(ProtocolHandler*) handler, uint16_t opcode)
{
    R(PacketTrilogy*) packet = packet_trilogy_create(protocol_handler_basic(handler), opcode, 0);
    cs_trilogy_schedule_packet(handler, packet);
}

static void cs_trilogy_handle_op_login_info(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    R(Basic*) basic;
    R(const char*) account;
    R(const char*) sessionKey;
    
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
    
    printf("account: %s, sessionKey: %s\n", account, sessionKey);
    
    char_select_handle_unauthed_client((CharSelect*)protocol_handler_basic(handler), client);
}

void cs_client_trilogy_on_auth(R(CharSelectClient*) client)
{
    R(ProtocolHandler*) handler = char_select_client_handler(client);
    R(Basic*) basic             = protocol_handler_basic(handler);
    Aligned w;
    R(PacketTrilogy*) packet;
    
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

static void cs_client_trilogy_characters_callback(R(Query*) query)
{
    R(CharSelectClient*) client = query_userdata_type(query, CharSelectClient);
    R(ProtocolHandler*) handler = char_select_client_handler(client);
    R(PacketTrilogy*) packet;
    R(CSTrilogy_CharSelectInfo*) cs;
    uint32_t i;
    
    /*
        This packet is about the worst case scenario for using the Aligned writer...
        would need to jump around a lot, backwards and forwards. So instead of that, we'll just
        do all writes in terms of memcpy, snprintf or single bytes to ensure they are aligned.
    */
    
    packet  = packet_trilogy_create(protocol_handler_basic(handler), TrilogyOp_CharacterInfo, sizeof(CSTrilogy_CharSelectInfo));
    cs      = (CSTrilogy_CharSelectInfo*)packet_trilogy_data(packet);
    
    memset(cs, 0, sizeof(CSTrilogy_CharSelectInfo));
    
    for (i = 0; i < 10; i++)
    {
        memset(cs->weirdA[i], i + 1, sizeof(uint32_t));
        memset(cs->weirdB[i], i + 1, sizeof(uint32_t));
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
        // since they don't fit into the uint8_t's provided by the struct
        
        client->weaponMaterialsTrilogy[i][0] = (uint32_t)query_get_int64(query, 15);
        client->weaponMaterialsTrilogy[i][1] = (uint32_t)query_get_int64(query, 16);
        
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

void cs_client_trilogy_on_account_id(R(CharSelectClient*) client, uint32_t accountId)
{
    R(Database*) db = core_db(C(protocol_handler_basic(char_select_client_handler(client))));
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
        "WHERE fk_account_id = ? "
        "ORDER BY character_id ASC LIMIT 10",
        cs_client_trilogy_characters_callback);
    
    query_bind_int64(&query, 1, (int64_t)accountId);
    
    db_schedule(db, &query);
}

static void cs_trilogy_handle_op_guild_list(R(CharSelectClient*) client, R(ProtocolHandler*) handler)
{
    Aligned write;
    R(Aligned*) w = &write;
    R(PacketTrilogy*) packet;
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

static void cs_trilogy_handle_op_name_approval(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    R(const char*) name;
    uint32_t race;
    uint32_t class;
    
    printf("NameApproval: %u\n", aligned_remaining(a));
    if (!char_select_client_is_authed(client) || aligned_remaining(a) < sizeof(CSTrilogy_NameApproval))
        return;
    
    name    = (const char*)aligned_current(a);
    aligned_advance(a, sizeof_field(CSTrilogy_NameApproval, name));
    race    = aligned_read_uint32(a);
    class   = aligned_read_uint32(a);
    
    printf("Name approval: %s, race %u, class %u\n", name, race, class);
    
    char_select_client_query_character_name_taken(client, (CharSelect*)protocol_handler_basic(handler), name);
}

void cs_client_trilogy_on_character_name_checked(R(CharSelectClient*) client, int taken)
{
    R(ProtocolHandler*) handler = char_select_client_handler(client);
    R(PacketTrilogy*) packet    = packet_trilogy_create(protocol_handler_basic(handler), TrilogyOp_NameApproval, 1);
    int available               = !taken;
    
    if (available)
        char_select_client_set_name_approved(client, true);
    
    packet_trilogy_data(packet)[0] = (uint8_t)available;
    cs_trilogy_schedule_packet(handler, packet);
}

static int cs_client_trilogy_char_creation_params_are_valid(R(CSTrilogy_CharCreateParams*) params)
{
    int class   = params->class - 1;
    int race    = params->race - 1;
    
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
    static uint16_t classesByRace[13] = {
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
    
    //fixme: add stat value checks
    
    // Iksar index correction
    if (race > 12)
        race = 12;
    
    if (class < 0 || class >= 14 || (classesByRace[race] & (1 << class)) == 0)
        return false;
    
    return true;
}

static void cs_client_trilogy_send_char_create_failure(R(CharSelectClient*) client)
{
    R(ProtocolHandler*) handler = char_select_client_handler(client);
    R(PacketTrilogy*) packet    = packet_trilogy_create(protocol_handler_basic(handler), TrilogyOp_NameApproval, 1);
    
    char_select_client_set_name_approved(client, false);
    
    packet_trilogy_data(packet)[0] = 0;
    cs_trilogy_schedule_packet(handler, packet);
}

static void cs_client_trilogy_create_character_callback(R(Query*) query)
{
    R(CharSelectClient*) client = query_userdata_type(query, CharSelectClient);
    
    if (query_affected_rows(query) == 0)
        cs_client_trilogy_send_char_create_failure(client);
    else
        cs_client_trilogy_on_account_id(client, char_select_client_account_id(client));
    
    char_select_client_drop(client);
}

static void cs_trilogy_handle_op_create_character(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    CSTrilogy_CharCreateParams params;
    R(Database*) db;
    Query query;
    
    printf("CreateCharacter %u vs %lu\n", aligned_remaining(a), sizeof(CSTrilogy_CreateCharacter));
    
    if (!char_select_client_is_authed(client) || !char_select_client_is_name_approved(client) || aligned_remaining(a) < sizeof(CSTrilogy_CreateCharacter))
        return;
    
    params.accountId = char_select_client_account_id(client);
    
    // name
    snprintf(params.name, sizeof(params.name), "%s", (const char*)aligned_current(a));
    aligned_advance(a, sizeof_field(CSTrilogy_CreateCharacter, name) + sizeof_field(CSTrilogy_CreateCharacter, surname));
    // gender
    params.gender       = aligned_read_uint16(a);
    // race
    params.race         = aligned_read_uint16(a);
    // class
    params.class        = aligned_read_uint16(a);
    // level, experience, trainingPoints, currentMana
    aligned_advance(a, sizeof(uint32_t) * 2 + sizeof(uint16_t) * 2);
    // face
    params.face         = aligned_read_uint8(a);
    // unknownA[47]
    aligned_advance(a, sizeof(uint8_t) * 47);
    // currentHp
    params.currentHp    = aligned_read_int16(a);
    // unknownB
    aligned_advance(a, sizeof(uint8_t));
    // STR
    params.STR          = aligned_read_uint8(a);
    // STA
    params.STA          = aligned_read_uint8(a);
    // CHA
    params.CHA          = aligned_read_uint8(a);
    // DEX
    params.DEX          = aligned_read_uint8(a);
    // INT
    params.INT          = aligned_read_uint8(a);
    // AGI
    params.AGI          = aligned_read_uint8(a);
    // WIS
    params.WIS          = aligned_read_uint8(a);
    // stuff we don't care about
    aligned_advance(a, sizeof_field(CSTrilogy_CreateCharacter, stuffCharSelectDoesntCareAboutA));
    // deity
    params.deity        = aligned_read_uint16(a);
    
    params.zoneId       = 54; //fixme: determine this from somewhere (lua script?)
    params.x            = 0.0f;
    params.y            = 0.0f;
    params.z            = 0.0f;
    
    // Verify that the given character creation parameters are valid
    if (!cs_client_trilogy_char_creation_params_are_valid(&params))
    {
        cs_client_trilogy_send_char_create_failure(client);
        return;
    }
    
    char_select_client_grab(client);
    
    db = core_db(C(protocol_handler_basic(handler)));
    
    query_init(&query);
    query_set_userdata(&query, client);
    db_prepare_literal(db, &query,
        "INSERT OR IGNORE INTO character " // Why OR IGNORE? Just in case two people try to make a character with the same name at the same time...
            "(fk_account_id, name, gender, race, class, face, current_hp, base_str, base_sta, base_cha, base_dex, base_int, base_agi, base_wis, deity, zone_id, x, y, z) "
        "VALUES "
            "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        cs_client_trilogy_create_character_callback);
    
    // accountId
    query_bind_int64(&query, 1, (int64_t)params.accountId);
    // name
    query_bind_string(&query, 2, params.name, -1);
    // gender
    query_bind_int(&query, 3, params.gender);
    // race
    query_bind_int(&query, 4, params.race);
    // class
    query_bind_int(&query, 5, params.class);
    // face
    query_bind_int(&query, 6, params.face);
    // currentHp
    query_bind_int(&query, 7, params.currentHp);
    // STR
    query_bind_int(&query, 8, params.STR);
    // STA
    query_bind_int(&query, 9, params.STA);
    // CHA
    query_bind_int(&query, 10, params.CHA);
    // DEX
    query_bind_int(&query, 11, params.DEX);
    // INT
    query_bind_int(&query, 12, params.INT);
    // AGI
    query_bind_int(&query, 13, params.AGI);
    // WIS
    query_bind_int(&query, 14, params.WIS);
    // deity
    query_bind_int(&query, 15, params.deity);
    // zoneId
    query_bind_int(&query, 16, params.zoneId);
    // x
    query_bind_double(&query, 17, params.x);
    // y
    query_bind_double(&query, 18, params.y);
    // z
    query_bind_double(&query, 19, params.z);
    
    db_schedule(db, &query);
}

static void cs_trilogy_handle_op_delete_character(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    R(const char*) name;
    
    printf("DeleteCharacter %u\n", aligned_remaining(a));
    if (!char_select_client_is_authed(client) || aligned_remaining(a) < 2)
        return;
    
    name = (const char*)aligned_current(a);
    printf("Delete: %s\n", name);
    
    char_select_client_delete_character_by_name(client, (CharSelect*)protocol_handler_basic(handler), name);
}

static void cs_trilogy_handle_op_wear_change(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    char buf[2048];
    int p;
    
    printf("WearChange:\n");
    if (!char_select_client_is_authed(client))
        return;
    
    p = snprintf(buf, sizeof(buf), "WearChange, len %u, data:\n", aligned_remaining(a));
    
    while (aligned_remaining(a))
    {
        p += snprintf(buf + p, sizeof(buf) - p, "%02x ", aligned_read_byte(a));
    }
    
    log_format(protocol_handler_basic(handler), LogNetwork, "%s", buf);
}

static void cs_trilogy_handle_op_enter(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    R(const char*) name;

    (void)handler;
    
    printf("Enter %u\n", aligned_remaining(a));
    if (!char_select_client_is_authed(client) || aligned_remaining(a) < 2)
        return;
    
    name = (const char*)aligned_current(a);
    printf("Enter: %s\n", name);
}

void client_recv_packet_trilogy(R(void*) vclient, uint16_t opcode, R(Aligned*) a)
{
    R(CharSelectClient*) client = (CharSelectClient*)vclient;
    R(ProtocolHandler*) handler = char_select_client_handler(client);

    printf("Received packet opcode 0x%04x, length %u:\n", opcode, aligned_remaining(a));
    
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
