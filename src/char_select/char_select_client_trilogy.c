
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

static void cs_trilogy_handle_op_login_info(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    R(Basic*) basic;
    R(const char*) account;
    R(const char*) sessionKey;
    Aligned write;
    R(Aligned*) w = &write;
    R(PacketTrilogy*) packet;
    R(CSTrilogy_CharSelectInfo*) cs;
    uint32_t i;
    
    (void)client;
    
    if (aligned_remaining(a) == 0)
        return;
    
    account = (const char*)aligned_current(a);
    
    aligned_advance_null_terminator(a);
    
    if (aligned_remaining(a) == 0)
        return;
    
    sessionKey  = (const char*)aligned_current(a);
    basic       = protocol_handler_basic(handler);
    
    printf("account: %s, sessionKey: %s\n", account, sessionKey);
    
    aligned_set_basic(w, basic);
    
    packet = packet_trilogy_create(basic, TrilogyOp_LoginApproved, 1);
    packet_trilogy_data(packet)[0] = 0;
    cs_trilogy_schedule_packet(handler, packet);
    
    packet = packet_trilogy_create(basic, TrilogyOp_Enter, 1);
    packet_trilogy_data(packet)[0] = 0;
    cs_trilogy_schedule_packet(handler, packet);
    
    packet = packet_trilogy_create(basic, TrilogyOp_ExpansionInfo, sizeof(uint32_t));
    aligned_reinit(w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    aligned_write_uint32(w, 1 | 2); // Bitfield: 1 = Kunark, 2 = Velious, 4 = Luclin
    cs_trilogy_schedule_packet(handler, packet);
    
    packet = packet_trilogy_create(basic, TrilogyOp_CharacterInfo, sizeof(CSTrilogy_CharSelectInfo));
    cs = (CSTrilogy_CharSelectInfo*)packet_trilogy_data(packet);
    
    memset(cs, 0, sizeof(CSTrilogy_CharSelectInfo));
    
    for (i = 0; i < 10; i++)
    {
        snprintf(cs->names[i], 30, "<none>");
        snprintf(cs->zoneShortNames[i], 20, "qeynos");
    }
    
    for (i = 0; i < 8; i++)
    {
        memset(cs->weirdA[i], i + 1, sizeof(uint32_t));
        memset(cs->weirdB[i], i + 1, sizeof(uint32_t));
    }
    
    cs_trilogy_schedule_packet(handler, packet);
}

static void cs_trilogy_handle_op_guild_list(R(CharSelectClient*) client, R(ProtocolHandler*) handler, R(Aligned*) a)
{
    R(PacketTrilogy*) packet;
    R(CSTrilogy_GuildList*) list;
    uint32_t i;
    (void)client;
    
    printf("Guild list: %u\n", aligned_read_uint32(a));
    
    // The entire guild list struct is properly aligned for its 4-byte fields
    packet  = packet_trilogy_create(protocol_handler_basic(handler), TrilogyOp_GuildList, sizeof(CSTrilogy_GuildList));
    list    = (CSTrilogy_GuildList*)packet_trilogy_data(packet);
    
    memset(list, 0, sizeof(CSTrilogy_GuildList));
    
    for (i = 0; i < EQP_CHAR_SELECT_MAX_GUILDS; i++)
    {
        R(CSTrilogy_GuildEntry*) guild = &list->guilds[i];
        
        guild->guildId  = 0xffffffff;
        guild->unknownA = 0xffffffff;
        guild->unknownC = 0xffffffff;
    }
    
    cs_trilogy_schedule_packet(handler, packet);
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
        cs_trilogy_handle_op_guild_list(client, handler, a);
        break;
    
    default:
        break;
    }
}
