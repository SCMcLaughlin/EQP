
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
}

void cs_client_trilogy_on_account_id(R(CharSelectClient*) client, uint32_t accountId)
{
    R(Database*) db = core_db(C(protocol_handler_basic(char_select_client_handler(client))));
    Query query;
    
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

static void cs_trilogy_handle_op_guild_list(R(ProtocolHandler*) handler)
{
    Aligned write;
    R(Aligned*) w = &write;
    R(PacketTrilogy*) packet;
    uint32_t i;
    
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
    (void)client;
    (void)handler;
    (void)a;
}

static void cs_trilogy_handle_op_create_character(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    (void)client;
    (void)handler;
    (void)a;
}

static void cs_trilogy_handle_op_delete_character(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    (void)client;
    (void)handler;
    (void)a;
}

static void cs_trilogy_handle_op_wear_change(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    (void)client;
    (void)handler;
    (void)a;
}

static void cs_trilogy_handle_op_enter(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    (void)client;
    (void)handler;
    (void)a;
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
        cs_trilogy_handle_op_guild_list(handler);
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
