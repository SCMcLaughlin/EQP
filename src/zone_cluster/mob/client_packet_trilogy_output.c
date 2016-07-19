
#include "client_packet_trilogy_output.h"
#include "zone_cluster.h"

void client_trilogy_schedule_packet_individual(Client* client, PacketTrilogy* packet)
{
    ProtocolHandler* handler = client_handler(client);
    packet_trilogy_fragmentize(packet);
    protocol_handler_trilogy_schedule_packet(&handler->trilogy, packet);
}

void client_trilogy_schedule_packet_broadcast(Client* client, PacketTrilogy* packet)
{
    ProtocolHandler* handler = client_handler(client);
    packet_trilogy_grab(packet);
    protocol_handler_trilogy_schedule_packet(&handler->trilogy, packet);
}

void client_trilogy_send_zero_length_packet(Client* client, uint16_t opcode)
{
    PacketTrilogy* packet = packet_trilogy_create(B(client_zone_cluster(client)), opcode, 0);
    client_trilogy_schedule_packet_individual(client, packet);
}

void client_trilogy_send_zeroed_packet_var_length(Client* client, uint16_t opcode, uint32_t length)
{
    PacketTrilogy* packet = packet_trilogy_create(B(client_zone_cluster(client)), opcode, length);
    memset(packet_trilogy_data(packet), 0, length);
    client_trilogy_schedule_packet_individual(client, packet);
}

void client_trilogy_send_keep_alive(ProtocolHandler* handler)
{
    protocol_handler_trilogy_send_keep_alive_ack(&handler->trilogy);
}

static void client_trilogy_player_profile_obfuscate(byte* buffer, uint32_t len)
{
    uint32_t* ptr   = (uint32_t*)buffer;
    uint32_t cur    = 0x65e7;
    uint32_t n      = len / sizeof(uint32_t);
    uint32_t next;
    uint32_t i;
    
    i       = len / 8;
    next    = ptr[0];
    ptr[0]  = ptr[i];
    ptr[i]  = next;
    
    for (i = 0; i < n; i++)
    {
        next    = cur + ptr[i] - 0x37a9;
        ptr[i]  = ((ptr[i] << 0x07) | (ptr[i] >> 0x19)) + 0x37a9;
        ptr[i]  = ((ptr[i] << 0x0f) | (ptr[i] >> 0x11));
        ptr[i]  = ptr[i] - cur;
        cur     = next;
    }
}

static void client_trilogy_send_player_profile_compress_and_obfuscate(Client* client, Trilogy_PlayerProfile* pp)
{
    byte buffer[sizeof(Trilogy_PlayerProfile)];
    unsigned long length    = sizeof(buffer);
    ZC* zc                  = client_zone_cluster(client);
    PacketTrilogy* packet;
    int rc;
    
    rc = compress2(buffer, &length, (const byte*)pp, sizeof(Trilogy_PlayerProfile), Z_BEST_COMPRESSION);
    
    if (rc != Z_OK)
        exception_throw_format(B(zc), ErrorCompression, "[client_trilogy_send_player_profile_compress_and_obfuscate] ZLib compression failed, err code %i", rc);
    
    client_trilogy_player_profile_obfuscate(buffer, length);
    
    packet = packet_trilogy_create(B(zc), TrilogyOp_PlayerProfile, length);
    memcpy(packet_trilogy_data(packet), buffer, length);
    
    client_trilogy_schedule_packet_individual(client, packet);
}

static void temp_write_packet(const char* leader, const byte* packet, uint32_t len)
{
    uint32_t i, j, k;
    
    static FILE* fp = NULL;
    
    if (fp == NULL)
    {
        fp = fopen("packet.txt", "a");
        setvbuf(fp, NULL, _IONBF, 0);
    }
    
    fprintf(fp, "%s:\n", leader);
    
    i = 0;
    while (i < len)
    {
        fprintf(fp, "[%04u] ", i);
        
        j = len - i;
        if (j > 8) j = 8;
        
        for (k = 0; k < j; k++) fprintf(fp, "%02x ", packet[i + k]);
        fprintf(fp, "   ");
        for (k = 0; k < j; k++) fputc(isprint(packet[i + k]) ? packet[i + k] : '.', fp);
        fputc('\n', fp);
        
        i += 8;
    }
    
    fprintf(fp, "\n\n");
}

void client_trilogy_send_player_profile(Client* client)
{
    Trilogy_PlayerProfile pp;
    Aligned write;
    Aligned* w = &write;
    InventoryIterator invItr;
    Inventory* inv  = &client->inventory;
    uint64_t time   = clock_milliseconds();
    uint32_t reset;
    uint32_t i;
    
    aligned_init(B(client_zone_cluster(client)), w, &pp, sizeof(pp));
    
    aligned_write_zero_all(w);
    
    // crc -- gets written last
    aligned_advance(w, sizeof(uint32_t));
    // name
    aligned_write_snprintf_full_advance(w, sizeof(pp.name), "%s", client_name_cstr(client));
    // surname
    if (client->surname)
        aligned_write_snprintf_full_advance(w, sizeof(pp.surname), "%s", string_data(client->surname));
    else
        aligned_advance(w, sizeof(pp.surname));
    // gender
    aligned_write_uint16(w, client_base_gender(client));
    // race
    aligned_write_uint16(w, client_base_race(client));
    // class
    aligned_write_uint16(w, client_class(client));
    // level
    aligned_write_uint32(w, client_level(client));
    // experience
    aligned_write_uint32(w, client->experience);
    // trainingPoints
    aligned_write_uint16(w, client->trainingPoints);
    // currentMana
    aligned_write_int16(w, client_current_mana(client));
    // face
    aligned_write_uint8(w, client_face(client));
    // unknownA[47]
    aligned_advance(w, sizeof(pp.unknownA));
    // currentHp
    aligned_write_int16(w, client_current_hp(client));
    // unknownB
    aligned_advance(w, sizeof(pp.unknownB));
    // STR
    aligned_write_uint8(w, client_base_str(client));
    // STA
    aligned_write_uint8(w, client_base_sta(client));
    // CHA
    aligned_write_uint8(w, client_base_cha(client));
    // DEX
    aligned_write_uint8(w, client_base_dex(client));
    // INT
    aligned_write_uint8(w, client_base_int(client));
    // AGI
    aligned_write_uint8(w, client_base_agi(client));
    // WIS
    aligned_write_uint8(w, client_base_wis(client));
    
    // languages
    for (i = 0; i < EQP_TRILOGY_LANGUAGES_COUNT; i++)
        aligned_write_uint8(w, client->skills.language[i]);
        
    // unknownC[14]
    aligned_advance(w, sizeof(pp.unknownC));
    
    // mainInventoryItemIds
    aligned_write_memset_no_advance(w, 0xff, sizeof(pp.mainInventoryItemIds));
    reset = aligned_position(w);
    
    inventory_iterator_init(&invItr, InvSlot_EquipMainAndCursorNoCharmBegin, InvSlot_EquipMainAndCursorNoCharmEnd);
    
    while (inventory_iterate_no_augs(inv, &invItr))
    {
        InventorySlot* slot = invItr.slot;
        
        aligned_advance(w, sizeof(uint16_t) * (slot->slotId - 1)); // No charm slot
        aligned_write_uint16(w, slot->itemId);
        aligned_reset_to(w, reset);
    }
    
    aligned_advance(w, sizeof(pp.mainInventoryItemIds));
    
    // mainInventoryInternalUnused
    aligned_advance(w, sizeof(pp.mainInventoryInternalUnused));
    
    // mainInventoryItemProperties
    reset = aligned_position(w);
    
    for (i = 0; i < 30; i++)
    {
        aligned_advance(w, sizeof(uint32_t));
        aligned_write_uint16(w, 0xffff);
        aligned_advance(w, sizeof(uint32_t));
    }
    
    aligned_reset_to(w, reset);
    inventory_iterator_init(&invItr, InvSlot_EquipMainAndCursorNoCharmBegin, InvSlot_EquipMainAndCursorNoCharmEnd);
    
    while (inventory_iterate_no_augs(inv, &invItr))
    {
        InventorySlot* slot = invItr.slot;
        
        aligned_advance(w, sizeof(Trilogy_PPItem) * (slot->slotId - 1)); // No charm slot
        
        aligned_advance(w, sizeof(uint16_t));
        aligned_write_uint8(w, item_get_charges(slot->item));
        
        aligned_reset_to(w, reset);
    }
    
    aligned_advance(w, sizeof(pp.mainInventoryItemProperties));
    
    // buffs
    //fixme: do these properly once buffs are stored somewhere
    for (i = 0; i < EQP_TRILOGY_MAX_BUFFS; i++)
    {
        aligned_advance(w, sizeof(uint32_t));
        aligned_write_uint16(w, 0xffff);    // spellId = none
        aligned_advance(w, sizeof(uint32_t));
    }
    
    // baggedItemIds - these are entirely FF'd out when there is no bag in the corresponding main inventory/cursor slot
    aligned_write_memset_no_advance(w, 0xff, sizeof(pp.baggedItemIds));
    reset = aligned_position(w);
    
    inventory_iterator_init(&invItr, InvSlot_BagsSlotsIncludingCursorBegin, InvSlot_BagsSlotsIncludingCursorEnd);
    
    while (inventory_iterate_no_augs(inv, &invItr))
    {
        InventorySlot* slot = invItr.slot;
        
        aligned_advance(w, sizeof(uint16_t) * (slot->slotId - InvSlot_BagsSlotsIncludingCursorBegin));
        aligned_write_uint16(w, slot->itemId);
        aligned_reset_to(w, reset);
    }
    
    aligned_advance(w, sizeof(pp.baggedItemIds));
    
    // baggedItemProperties
    reset = aligned_position(w);
    
    for (i = 0; i < 90; i++)
    {
        aligned_advance(w, sizeof(uint32_t));
        aligned_write_uint16(w, 0xffff);
        aligned_advance(w, sizeof(uint32_t));
    }
    
    aligned_reset_to(w, reset);
    inventory_iterator_init(&invItr, InvSlot_BagsSlotsIncludingCursorBegin, InvSlot_BagsSlotsIncludingCursorEnd);
    
    while (inventory_iterate_no_augs(inv, &invItr))
    {
        InventorySlot* slot = invItr.slot;
        
        aligned_advance(w, sizeof(Trilogy_PPItem) * (slot->slotId - InvSlot_BagsSlotsIncludingCursorBegin));
        
        aligned_advance(w, sizeof(uint16_t));
        aligned_write_uint8(w, item_get_charges(slot->item));
        
        aligned_reset_to(w, reset);
    }
    
    aligned_advance(w, sizeof(pp.baggedItemProperties));
    
    // spellbook
    aligned_write_memset_no_advance(w, 0xff, sizeof(pp.spellbook));
    
    if (!client_spellbook_is_empty(client))
    {
        SpellbookSlot* array    = array_data_type(client->spellbook.knownSpells, SpellbookSlot);
        uint32_t n              = array_count(client->spellbook.knownSpells);
        reset                   = aligned_position(w);
        
        for (i = 0; i < n; i++)
        {
            SpellbookSlot slot = array[i];
            
            if (slot.slotId >= EQP_TRILOGY_SPELLBOOK_SLOT_COUNT)
                continue;
            
            aligned_advance(w, sizeof(uint16_t) * slot.slotId);
            aligned_write_uint16(w, slot.spellId);
            
            aligned_reset_to(w, reset);
        }
    }
    
    aligned_advance(w, sizeof(pp.spellbook));
    
    // memmedSpellIds
    for (i = 0; i < EQP_TRILOGY_MEMMED_SPELL_SLOT_COUNT; i++)
    {
        uint16_t spellId = client->spellbook.memorized[i].spellId;
        
        aligned_write_uint16(w, spellId ? spellId : 0xffff);
    }
    
    // unknownD
    aligned_advance(w, sizeof(uint16_t));
    // y
    aligned_write_float(w, client_y(client));
    // x
    aligned_write_float(w, client_x(client));
    // z
    aligned_write_float(w, client_z(client));
    // heading
    aligned_write_float(w, client_heading(client));
    // zoneShortName
    aligned_write_snprintf_full_advance(w, sizeof(pp.zoneShortName), "%s", zone_get_short_name(client_zone(client)));
    // unknownEDefault100
    aligned_write_uint32(w, 100);
    // coins
    aligned_write_buffer(w, &client->coins, sizeof(pp.coins));
    // coinsBank
    aligned_write_buffer(w, &client->coinsBank, sizeof(pp.coinsBank));
    // coinsCursor
    aligned_write_buffer(w, &client->coinsCursor, sizeof(pp.coinsCursor));
    
    // skills
    for (i = 0; i < EQP_TRILOGY_SKILLS_COUNT; i++)
    {
        aligned_write_uint8(w, client->skills.skill[i]);
    }
    
    // unknownF[162]
    aligned_write_memset(w, 0xff, 27);
    aligned_advance(w, 1);
    aligned_write_uint32(w, 0xffffffff);
    aligned_advance(w, 1);
    aligned_write_memset(w, 0xff, 3);
    aligned_advance(w, 1);
    aligned_write_uint8(w, 0xff);
    aligned_advance(w, 1);
    aligned_write_memset(w, 0xff, 12);
    aligned_advance(w, 2);
    aligned_write_uint8(w, 0xff);
    aligned_advance(w, 2);
    aligned_write_uint8(w, 0x80);
    aligned_write_uint8(w, 0xbf);
    aligned_advance(w, 2);
    aligned_write_uint16(w, 0x4040);
    aligned_advance(w, 2);
    aligned_write_uint8(w, 0x20);
    aligned_write_uint8(w, 0x40);
    aligned_advance(w, 2);
    aligned_write_uint8(w, 0xb0);
    aligned_write_uint8(w, 0x40);
    aligned_advance(w, 92);
    
    // autoSplit
    aligned_write_uint32(w, client->isAutoSplitEnabled);
    // pvpEnabled
    aligned_write_uint32(w, client_is_pvp(client));
    // unknownG[12]
    aligned_advance(w, sizeof(pp.unknownG));
    // isGM
    aligned_write_uint32(w, client_is_gm(client));
    // unknownH[20]
    aligned_advance(w, sizeof(pp.unknownH));
    // disciplinesReady
    aligned_write_uint32(w, (time >= client->disciplineTimestamp));
    // unknownI[20]
    aligned_advance(w, sizeof(pp.unknownI));
    // hunger
    aligned_write_uint32(w, client->hungerLevel);
    // thirst
    aligned_write_uint32(w, client->thirstLevel);
    // unknownJ[24]
    aligned_advance(w, sizeof(pp.unknownJ));
    
    // bindZoneShortName[5]
    for (i = 0; i < 5; i++)
    {
        aligned_write_snprintf_full_advance(w, sizeof(pp.bindZoneShortName[i]), "%s", zone_short_name_by_id(client->bindPoints[i].zoneId));
    }
    
    // bankInventoryItemProperties
    reset = aligned_position(w);
    
    for (i = 0; i < 8; i++)
    {
        aligned_advance(w, sizeof(uint32_t));
        aligned_write_uint16(w, 0xffff);
        aligned_advance(w, sizeof(uint32_t));
    }
    
    aligned_reset_to(w, reset);
    inventory_iterator_init(&invItr, InvSlot_BankBegin, InvSlot_BankEnd);
    
    while (inventory_iterate_no_augs(inv, &invItr))
    {
        InventorySlot* slot = invItr.slot;
        
        aligned_advance(w, sizeof(Trilogy_PPItem) * (slot->slotId - InvSlot_BankBegin));
        
        aligned_advance(w, sizeof(uint16_t));
        aligned_write_uint8(w, item_get_charges(slot->item));

        aligned_reset_to(w, reset);
    }
    
    aligned_advance(w, sizeof(pp.bankInventoryItemProperties));
    
    // bankBaggedItemProperties
    reset = aligned_position(w);
    
    for (i = 0; i < 80; i++)
    {
        aligned_advance(w, sizeof(uint32_t));
        aligned_write_uint16(w, 0xffff);
        aligned_advance(w, sizeof(uint32_t));
    }
    
    aligned_reset_to(w, reset);
    inventory_iterator_init(&invItr, InvSlot_BankBagSlotsBegin, InvSlot_BankBagSlotsEnd);
    
    while (inventory_iterate_no_augs(inv, &invItr))
    {
        InventorySlot* slot = invItr.slot;
        
        aligned_advance(w, sizeof(Trilogy_PPItem) * (slot->slotId - InvSlot_BankBagSlotsBegin));
        
        aligned_advance(w, sizeof(uint16_t));
        aligned_write_uint8(w, item_get_charges(slot->item));
        
        aligned_reset_to(w, reset);
    }
    
    aligned_advance(w, sizeof(pp.bankBaggedItemProperties));
    
    // unknownK
    aligned_advance(w, sizeof(pp.unknownK));
    
    // bindLocY[5]
    aligned_write_float(w, client->bindPoints[0].loc.y);
    aligned_write_float(w, client->bindPoints[1].loc.y);
    aligned_write_float(w, client->bindPoints[2].loc.y);
    aligned_write_float(w, client->bindPoints[3].loc.y);
    aligned_write_float(w, client->bindPoints[4].loc.y);
    // bindLocX[5]
    aligned_write_float(w, client->bindPoints[0].loc.x);
    aligned_write_float(w, client->bindPoints[1].loc.x);
    aligned_write_float(w, client->bindPoints[2].loc.x);
    aligned_write_float(w, client->bindPoints[3].loc.x);
    aligned_write_float(w, client->bindPoints[4].loc.x);
    // bindLocZ[5]
    aligned_write_float(w, client->bindPoints[0].loc.z);
    aligned_write_float(w, client->bindPoints[1].loc.z);
    aligned_write_float(w, client->bindPoints[2].loc.z);
    aligned_write_float(w, client->bindPoints[3].loc.z);
    aligned_write_float(w, client->bindPoints[4].loc.z);
    // bindLocHeading[5]
    aligned_write_float(w, client->bindPoints[0].loc.heading);
    aligned_write_float(w, client->bindPoints[1].loc.heading);
    aligned_write_float(w, client->bindPoints[2].loc.heading);
    aligned_write_float(w, client->bindPoints[3].loc.heading);
    aligned_write_float(w, client->bindPoints[4].loc.heading);
    
    // unknownL
    aligned_advance(w, sizeof(pp.unknownL));
    // bankInventoryInternalUnused[8]
    aligned_advance(w, sizeof(pp.bankInventoryInternalUnused));
    // unknownM[12]
    aligned_advance(w, sizeof(pp.unknownM));
    // unixTimeA (fixme: is this creation time, current time?)
    aligned_write_uint32(w, client->creationTimestamp);
    // unknownN[8]
    aligned_advance(w, sizeof(pp.unknownN));
    // unknownODefault1
    aligned_write_uint32(w, 1);
    // unknownP[8]
    aligned_advance(w, sizeof(pp.unknownP));
    
    // bankInventoryItemIds
    aligned_write_memset_no_advance(w, 0xff, sizeof(pp.bankInventoryItemIds));
    reset = aligned_position(w);
    
    inventory_iterator_init(&invItr, InvSlot_BankBegin, InvSlot_BankEnd);
    
    while (inventory_iterate_no_augs(inv, &invItr))
    {
        InventorySlot* slot = invItr.slot;
        
        aligned_advance(w, sizeof(uint16_t) * (slot->slotId - InvSlot_BankBegin));
        aligned_write_uint16(w, slot->itemId);
        aligned_reset_to(w, reset);
    }
    
    aligned_advance(w, sizeof(pp.bankInventoryItemIds));
    
    // bankBaggedItemIds
    aligned_write_memset_no_advance(w, 0xff, sizeof(pp.bankBaggedItemIds));
    reset = aligned_position(w);
    
    inventory_iterator_init(&invItr, InvSlot_BankBagSlotsBegin, InvSlot_BankBagSlotsEnd);
    
    while (inventory_iterate_no_augs(inv, &invItr))
    {
        InventorySlot* slot = invItr.slot;
        
        aligned_advance(w, sizeof(uint16_t) * (slot->slotId - InvSlot_BankBagSlotsBegin));
        aligned_write_uint16(w, slot->itemId);
        aligned_reset_to(w, reset);
    }
    
    aligned_advance(w, sizeof(pp.bankBaggedItemIds));
    
    // deity
    aligned_write_uint16(w, client_deity(client));
    // guildId
    aligned_write_uint16(w, client->guildId ? client->guildId : 0xffff);
    // unixTimeB
    aligned_write_uint32(w, (uint32_t)time);
    // unknownQ[4]
    aligned_advance(w, sizeof(pp.unknownQ));
    // unknownRDefault7f7f
    aligned_write_uint16(w, 0x7f7f);
    // fatiguePercent
    aligned_write_uint8(w, 0); //fixme: handle this
    // unknownS
    aligned_advance(w, sizeof(pp.unknownS));
    // unknownTDefault1
    aligned_write_uint8(w, 1);
    // anon
    aligned_write_uint16(w, client->anonSetting);
    // guildRank
    aligned_write_uint8(w, client->guildId ? client->guildRank : 0xff);
    // drunkeness
    aligned_write_uint8(w, client->drunkeness);
    // showEqLoadScreen
    aligned_advance(w, sizeof(pp.showEqLoadScreen));
    // unknownU
    aligned_advance(w, sizeof(pp.unknownU));
    
    // spellGemRefreshMilliseconds
    for (i = 0; i < EQP_TRILOGY_MEMMED_SPELL_SLOT_COUNT; i++)
    {
        uint64_t timestamp = client->spellbook.memorized[i].recastTimestamp;
        
        aligned_write_uint32(w, (time > timestamp) ? 0 : timestamp - time);
    }
    
    // unknownV
    aligned_advance(w, sizeof(pp.unknownV));
    // harmtouchRefreshMilliseconds
    aligned_write_uint32(w, (time > client->harmtouchTimestamp) ? 0 : client->harmtouchTimestamp - time);
    
    // groupMemberNames
    //fixme: handle these
    for (i = 0; i < 5; i++)
    {
        aligned_advance(w, sizeof(pp.groupMember[i]));
    }
    
    // unknownW
    aligned_advance(w, 70);
    aligned_write_uint16(w, 0xffff);
    aligned_advance(w, 6);
    aligned_write_memset(w, 0xff, 6);
    aligned_advance(w, 176);
    aligned_write_uint16(w, 0xffff);
    aligned_advance(w, 2);
    
    // The rest is already zeroed, don't need to bother with it

    // crc
    pp.crc = ~crc_calc32(((byte*)&pp) + sizeof(uint32_t), sizeof(pp) - sizeof(uint32_t));
    
    client_trilogy_send_player_profile_compress_and_obfuscate(client, &pp);
}

void client_trilogy_send_zone_entry(Client* client)
{
    PacketTrilogy* packet;
    Aligned write;
    Aligned* w  = &write;
    ZC* zc      = client_zone_cluster(client);
    
    packet = packet_trilogy_create(B(zc), TrilogyOp_ZoneEntry, sizeof(Trilogy_ZoneEntry));
    aligned_init(B(zc), w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    aligned_write_zero_all(w);
    
    // crc -- gets written last
    aligned_advance(w, sizeof(uint32_t));
    // unknownA
    aligned_advance(w, sizeof(uint8_t));
    // name
    aligned_write_snprintf_full_advance(w, sizeof_field(Trilogy_ZoneEntry, name), "%s", client_name_cstr(client));
    // zoneShortName
    aligned_write_snprintf_full_advance(w, sizeof_field(Trilogy_ZoneEntry, zoneShortName), "%s", zone_get_short_name(client_zone(client)));
    // unknownB
    aligned_advance(w, sizeof(uint8_t));
    // y
    aligned_write_float(w, client_y(client));
    // x
    aligned_write_float(w, client_x(client));
    // z
    aligned_write_float(w, client_z(client));
    // heading
    aligned_write_float(w, client_heading(client) * 2); //fixme: ?
    // unknownC
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownC));
    // guildId
    aligned_write_uint16(w, client->guildId ? client->guildId : 0xffff);
    // unknownD
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownD));
    // class
    aligned_write_uint8(w, client_class(client));
    // race
    aligned_write_uint16(w, client_base_race(client));
    // gender
    aligned_write_uint8(w, client_base_gender(client));
    // level
    aligned_write_uint8(w, client_level(client));
    // unknownE
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownE));
    // isPvP
    aligned_write_uint8(w, client_is_pvp(client));
    // unknownF
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownF));
    // face
    aligned_write_uint8(w, client_face(client));
    // helmMaterial
    aligned_write_uint8(w, client_helm_texture(client)); //fixme: is this right
    // unknownG
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownG));
    // secondaryWeaponModelId
    aligned_write_uint8(w, client_secondary_model_id(client));
    // primaryWeaponModelId
    aligned_write_uint8(w, client_primary_model_id(client));
    // unknownH
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownH));
    // helmColor
    aligned_write_uint32(w, 0); //fixme: handle this
    // unknownI
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownI));
    // texture
    aligned_write_uint8(w, client_texture(client));
    // unknownJ
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownJ));
    // walkingSpeed
    aligned_write_float(w, client_base_walking_speed(client));
    // runningSpeed
    aligned_write_float(w, client_base_running_speed(client));
    // unknownK
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownK));
    // anon
    aligned_write_uint8(w, client->anonSetting);
    // unknownL
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownL));
    // surname
    if (client->surname)
        aligned_write_snprintf_full_advance(w, sizeof_field(Trilogy_ZoneEntry, surname), "%s", string_data(client->surname));
    else
        aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, surname));
    // unknownM
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownM));
    // deity
    aligned_write_uint16(w, client_deity(client));
    
    // unknownN is already zeroed, don't need to bother with it
    
    // crc
    aligned_reset(w);
    aligned_write_uint32(w, ~crc_calc32(aligned_current(w) + sizeof(uint32_t), aligned_size(w) - sizeof(uint32_t)));
    
    client_trilogy_schedule_packet_individual(client, packet);
}

PacketTrilogy* client_trilogy_make_op_weather(ZC* zc, int weatherType, int intensity)
{
    PacketTrilogy* packet = packet_trilogy_create(B(zc), TrilogyOp_Weather, sizeof(Trilogy_Weather));
    Aligned w;
    
    aligned_init(B(zc), &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    // type
    aligned_write_int(&w, weatherType);
    // intensity
    aligned_write_int(&w, intensity);
    
    return packet;
}

PacketTrilogy* client_trilogy_make_op_zone_info(ZC* zc, Zone* zone)
{
    PacketTrilogy* packet = packet_trilogy_create(B(zc), TrilogyOp_ZoneInfo, sizeof(Trilogy_ZoneInfo));
    Aligned write;
    Aligned* w = &write;
    
    aligned_init(B(zc), w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    aligned_write_zero_all(w);
    
    // characterName (unused)
    aligned_advance(w, sizeof_field(Trilogy_ZoneInfo, characterName));
    // zoneShortName
    aligned_write_snprintf_full_advance(w, sizeof_field(Trilogy_ZoneInfo, zoneShortName), "%s", zone_get_short_name(zone));
    // zoneLongName
    aligned_write_snprintf_full_advance(w, sizeof_field(Trilogy_ZoneInfo, zoneLongName), "%s", zone_get_long_name(zone));
    // zoneType
    aligned_write_uint8(w, zone_type(zone));
    
    // fogRed[4]
    aligned_write_uint8(w, zone->fogStats[0].red);
    aligned_write_uint8(w, zone->fogStats[1].red);
    aligned_write_uint8(w, zone->fogStats[2].red);
    aligned_write_uint8(w, zone->fogStats[3].red);
    // fogGreen[4]
    aligned_write_uint8(w, zone->fogStats[0].green);
    aligned_write_uint8(w, zone->fogStats[1].green);
    aligned_write_uint8(w, zone->fogStats[2].green);
    aligned_write_uint8(w, zone->fogStats[3].green);
    // fogBlue[4]
    aligned_write_uint8(w, zone->fogStats[0].blue);
    aligned_write_uint8(w, zone->fogStats[1].blue);
    aligned_write_uint8(w, zone->fogStats[2].blue);
    aligned_write_uint8(w, zone->fogStats[3].blue);
    
    // unknownA
    aligned_advance(w, sizeof_field(Trilogy_ZoneInfo, unknownA));
    
    // fogMinClippingDistance[4]
    aligned_write_float(w, zone->fogStats[0].minClippingDistance);
    aligned_write_float(w, zone->fogStats[1].minClippingDistance);
    aligned_write_float(w, zone->fogStats[2].minClippingDistance);
    aligned_write_float(w, zone->fogStats[3].minClippingDistance);
    // fogMaxClippingDistance[4]
    aligned_write_float(w, zone->fogStats[0].maxClippingDistance);
    aligned_write_float(w, zone->fogStats[1].maxClippingDistance);
    aligned_write_float(w, zone->fogStats[2].maxClippingDistance);
    aligned_write_float(w, zone->fogStats[3].maxClippingDistance);
    
    // gravity
    aligned_write_float(w, zone_gravity(zone));
    
    // unknownB[50]
    aligned_write_uint8(w, 0x20);
    aligned_write_memset(w, 0x0a, 4);
    aligned_write_uint8(w, 0x18);
    aligned_write_uint8(w, 0x06);
    aligned_write_uint8(w, 0x02);
    aligned_write_uint8(w, 0x0a);
    aligned_advance(w, 8);
    aligned_write_memset(w, 0xff, 32);
    aligned_advance(w, 1);
    
    // skyType
    aligned_write_uint16(w, zone_sky_type(zone));
    
    // unknownC[8]
    aligned_write_uint8(w, 0x04);
    aligned_advance(w, 1);
    aligned_write_uint8(w, 0x02);
    aligned_advance(w, 5);
    
    // unknownD
    aligned_write_float(w, 0.75f);
    // safeSpotX
    aligned_write_float(w, zone->safeSpot.x);
    // safeSpotY
    aligned_write_float(w, zone->safeSpot.y);
    // safeSpotZ
    aligned_write_float(w, zone->safeSpot.z);
    // safeSpotHeading
    aligned_write_float(w, zone->safeSpot.heading);
    // minZ
    aligned_write_float(w, zone_min_z(zone));
    // minClippingDistance
    aligned_write_float(w, zone_min_clipping_distance(zone));
    // maxClippingDistance
    aligned_write_float(w, zone_max_clipping_distance(zone));
    
    // unknownD[32] is already zeroed, don't need to bother with it

    return packet;
}

PacketTrilogy* client_trilogy_make_op_spawn_appearance(ZC* zc, uint16_t entityId, uint16_t type, uint32_t value)
{
    PacketTrilogy* packet = packet_trilogy_create(B(zc), TrilogyOp_SpawnAppearance, sizeof(Trilogy_SpawnAppearance));
    Aligned write;
    Aligned* w = &write;
    
    aligned_init(B(zc), w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    // spawnId
    aligned_write_uint16(w, entityId);
    // unknownA
    aligned_write_uint16(w, 0);
    // type
    aligned_write_uint16(w, type);
    // unknownB
    aligned_write_uint16(w, 0);
    // value
    aligned_write_uint32(w, value);
    
    return packet;
}

static void client_trilogy_spawn_obfuscate(byte* data, uint32_t length)
{
    uint32_t* ptr   = (uint32_t*)data;
    uint32_t cur    = 0;
    uint32_t n      = length / sizeof(uint32_t);
    uint32_t i;
    uint32_t next;
    
    for (i = 0; i < n; i++)
    {
        next    = cur + ptr[i] - 0x65e7;
        ptr[i]  = ((ptr[i] << 0x09) | (ptr[i] >> 0x17)) + 0x65e7;
        ptr[i]  = ((ptr[i] << 0x0d) | (ptr[i] >> 0x13));
        ptr[i]  = ptr[i] - cur;
        cur     = next;
    }
}

PacketTrilogy* client_trilogy_make_op_spawn(ZC* zc, Mob* spawningMob)
{
    PacketTrilogy* packet = packet_trilogy_create(B(zc), TrilogyOp_Spawn, sizeof(Trilogy_Spawn));
    Aligned write;
    Aligned* w  = &write;
    int mobType = mob_get_type(spawningMob);
    int temp;
    
    aligned_init(B(zc), w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    aligned_write_zero_all(w);
    
    // unknownA
    aligned_advance(w, sizeof_field(Trilogy_Spawn, unknownA));
    // size
    aligned_write_float(w, mob_current_size(spawningMob));
    // walkingSpeed
    aligned_write_float(w, mob_current_walking_speed(spawningMob));
    // runningSpeed
    aligned_write_float(w, mob_current_running_speed(spawningMob));
    
    // tints
    aligned_write_uint32(w, mob_get_tint(spawningMob, 0));
    aligned_write_uint32(w, mob_get_tint(spawningMob, 1));
    aligned_write_uint32(w, mob_get_tint(spawningMob, 2));
    aligned_write_uint32(w, mob_get_tint(spawningMob, 3));
    aligned_write_uint32(w, mob_get_tint(spawningMob, 4));
    aligned_write_uint32(w, mob_get_tint(spawningMob, 5));
    aligned_write_uint32(w, mob_get_tint(spawningMob, 6));
    
    // unknownB
    aligned_advance(w, sizeof_field(Trilogy_Spawn, unknownB));
    // heading
    aligned_write_int8(w, spawningMob->headingRaw); //fixme: decide how to deal with headings
    // headingDelta
    aligned_advance(w, 1);
    // y
    aligned_write_int16(w, mob_y(spawningMob));
    // x
    aligned_write_int16(w, mob_x(spawningMob));
    // z
    aligned_write_int16(w, mob_z(spawningMob) * 10);
    // delta bitfield
    aligned_advance(w, sizeof(int));
    // unknownC
    aligned_advance(w, sizeof_field(Trilogy_Spawn, unknownC));
    // entityId
    aligned_write_uint16(w, mob_entity_id(spawningMob));
    // bodyType
    aligned_write_uint16(w, mob_body_type(spawningMob));
    // ownerEntityId
    aligned_write_uint16(w, mob_owner_entity_id(spawningMob));
    // hpPercent
    aligned_write_int16(w, mob_hp_ratio(spawningMob));
    
    // guildId
    if (mobType == MobType_Client)
    {
        uint16_t guildId = client_guild_id((Client*)spawningMob);
        aligned_write_uint16(w, guildId ? guildId : 0xffff);
    }
    else
    {
        aligned_write_uint16(w, 0xffff);
    }
    
    // race
    temp = mob_current_race(spawningMob);
    aligned_write_uint8(w, (temp > 0xff) ? 1 : temp);
    
    switch (mobType)
    {
    case MobType_Npc:
    case MobType_Pet:
        temp = 1;
        break;
    
    case MobType_Client:
        temp = 0;
        break;
    
    case MobType_NpcCorpse:
        temp = 3;
        break;
    
    case MobType_ClientCorpse:
        temp = 2;
        break;
    }
    
    // mobType
    aligned_write_uint8(w, temp);
    // class
    aligned_write_uint8(w, mob_class(spawningMob));
    // gender
    aligned_write_uint8(w, mob_current_gender(spawningMob));
    // level
    aligned_write_uint8(w, mob_level(spawningMob));
    // isInvisible
    aligned_write_uint8(w, mob_is_invisible(spawningMob));
    // unknownD
    aligned_advance(w, sizeof_field(Trilogy_Spawn, unknownD));
    
    // isPvP
    if (mobType == MobType_Client)
        aligned_write_uint8(w, client_is_pvp((Client*)spawningMob));
    else
        aligned_advance(w, sizeof(uint8_t));
    
    // uprightState
    aligned_write_uint8(w, mob_upright_state(spawningMob));
    // light
    aligned_write_uint8(w, mob_light_level(spawningMob));
    
    if (mobType == MobType_Client)
    {
        // anon
        aligned_write_uint8(w, client_anon_setting((Client*)spawningMob));
        // isAfk
        aligned_write_uint8(w, client_is_afk((Client*)spawningMob));
        // unknownE
        aligned_advance(w, sizeof_field(Trilogy_Spawn, unknownE));
        // isLinkdead
        aligned_write_uint8(w, client_is_linkdead((Client*)spawningMob));
        // isGM
        aligned_write_uint8(w, client_is_gm((Client*)spawningMob));
    }
    else
    {
        // anon, isAfk, unknownE, isLinkdead, isGM
        aligned_advance(w, 5);
    }
    
    // unknownF
    aligned_advance(w, sizeof_field(Trilogy_Spawn, unknownF));
    // texture
    aligned_write_uint8(w, mob_texture(spawningMob));
    // helmTexture
    aligned_write_uint8(w, mob_helm_texture(spawningMob));
    // unknownG
    aligned_advance(w, sizeof_field(Trilogy_Spawn, unknownG));
    
    // materials
    aligned_write_uint8(w, mob_get_material(spawningMob, 0));
    aligned_write_uint8(w, mob_get_material(spawningMob, 1));
    aligned_write_uint8(w, mob_get_material(spawningMob, 2));
    aligned_write_uint8(w, mob_get_material(spawningMob, 3));
    aligned_write_uint8(w, mob_get_material(spawningMob, 4));
    aligned_write_uint8(w, mob_get_material(spawningMob, 5));
    aligned_write_uint8(w, mob_get_material(spawningMob, 6));
    aligned_write_uint8(w, mob_primary_model_id(spawningMob));
    aligned_write_uint8(w, mob_secondary_model_id(spawningMob));
    
    // name
    aligned_write_snprintf_full_advance(w, sizeof_field(Trilogy_Spawn, name), "%s", mob_client_friendly_name_cstr(spawningMob));
    
    if (mobType == MobType_Client)
    {
        // surname
        aligned_write_snprintf_full_advance(w, sizeof_field(Trilogy_Spawn, surname), "%s", client_surname_cstr((Client*)spawningMob));
        // guildRank
        aligned_write_uint8(w, client_guild_rank((Client*)spawningMob));
    }
    else
    {
        // surname, guildRank
        aligned_advance(w, sizeof_field(Trilogy_Spawn, surname) + sizeof_field(Trilogy_Spawn, guildRank));
    }
    
    // unknownH
    aligned_advance(w, sizeof_field(Trilogy_Spawn, unknownH));
    // deity
    aligned_write_uint16(w, mob_deity(spawningMob));
    
    // unknownI is already zeroed, don't need to bother with it
    
    // Spawn packets are expected to be obfuscated
    client_trilogy_spawn_obfuscate(packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    return packet;
}

PacketTrilogy* client_trilogy_make_op_custom_message(ZC* zc, uint32_t chatChannel, const char* str, uint32_t len)
{
    PacketTrilogy* packet;
    Aligned w;
    
    len++; // Include null terminator
    
    packet = packet_trilogy_create(B(zc), TrilogyOp_CustomMessage, sizeof(Trilogy_CustomMessage) + len);
    aligned_init(B(zc), &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    // chatChannel
    aligned_write_uint32(&w, chatChannel);
    // message
    aligned_write_buffer(&w, str, len);
    
    return packet;
}

PacketTrilogy* client_trilogy_make_op_custom_message_format(ZC* zc, uint32_t chatChannel, const char* fmt, va_list args)
{
    char buf[4096];
    int len;
    
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    
    if (len < 0)
        exception_throw_literal(B(zc), ErrorFormatString, "[client_trilogy_make_op_custom_message_format] vsnprintf() failed");
    
    // snprintf returns the size it *attempted* to write, not the actual amount written (though if everything is good, these values are equal)
    if (len >= (int)sizeof(buf))
        len = sizeof(buf) - 1;
    
    return client_trilogy_make_op_custom_message(zc, chatChannel, buf, len);
}

PacketTrilogy* client_trilogy_make_item_packet(ZC* zc, uint16_t opcode, Item* item, uint16_t slotId)
{
    ItemPrototype* proto    = item_get_prototype(item);
    PacketTrilogy* packet   = packet_trilogy_create(B(zc), opcode, sizeof(Trilogy_Item));
    Aligned write;
    Aligned* w = &write;
    uint32_t i = 0;
    
    aligned_init(B(zc), w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    
    // name
    aligned_write_snprintf_full_advance(w, sizeof_field(Trilogy_Item, name), "%s", item_proto_get_name(proto));
    
    // The first character of the lore text controls certain item tags: '*' = lore, '&' = summoned, '#' = artifact, '~' = pending lore
    if (item_proto_is_lore(proto))
    {
        aligned_write_byte(w, '*');
        i = 1;
    }
    
    // lore
    aligned_write_snprintf_full_advance(w, sizeof_field(Trilogy_Item, lore) - i, "%s", item_proto_get_lore_text(proto));
    // model
    aligned_write_snprintf_full_advance(w, sizeof_field(Trilogy_Item, model), "IT%u", item_proto_get_model_id(proto));
    
    // typeFlag
    switch (item_proto_get_item_type_id(proto))
    {
    default:
        aligned_write_uint16(w, 0x3336);
        break;
    }
    
    // unknownA
    aligned_write_zeroes(w, sizeof_field(Trilogy_Item, unknownA));
    // weight
    aligned_write_uint8(w, item_proto_get_weight(proto));
    // isPermanent
    aligned_write_uint8(w, item_proto_is_permanent(proto) ? 255 : 0);
    // isDroppable
    aligned_write_uint8(w, item_proto_is_droppable(proto));
    // size
    aligned_write_uint8(w, item_proto_get_size(proto));
    // itemType
    aligned_write_uint8(w, 0); //fixme
    // itemId
    aligned_write_uint16(w, item_proto_get_item_id(proto));
    // icon
    aligned_write_uint16(w, item_proto_get_icon_id(proto));
    // currentSlot
    aligned_write_uint16(w, slotId);
    // slotsBitfield
    aligned_write_uint32(w, item_proto_get_slot_bitfield(proto));
    // cost
    aligned_write_uint32(w, item_proto_get_cost(proto));
    // unknownB
    aligned_write_zeroes(w, sizeof_field(Trilogy_Item, unknownB));
    // instanceId
    aligned_write_uint32(w, 0);
    // isDroppableRoleplayServer
    aligned_write_uint8(w, item_proto_is_droppable(proto));
    // unknownC
    aligned_write_zeroes(w, sizeof_field(Trilogy_Item, unknownC));
    
    // Split for basic vs book (fixme: implement books)
    
    // Basic
    
    // STR
    aligned_write_int8(w, item_proto_get_str(proto));
    // STA
    aligned_write_int8(w, item_proto_get_sta(proto));
    // CHA
    aligned_write_int8(w, item_proto_get_cha(proto));
    // DEX
    aligned_write_int8(w, item_proto_get_dex(proto));
    // INT
    aligned_write_int8(w, item_proto_get_int(proto));
    // AGI
    aligned_write_int8(w, item_proto_get_agi(proto));
    // WIS
    aligned_write_int8(w, item_proto_get_wis(proto));
    // MR
    aligned_write_int8(w, item_proto_get_sv_magic(proto));
    // FR
    aligned_write_int8(w, item_proto_get_sv_fire(proto));
    // CR
    aligned_write_int8(w, item_proto_get_sv_cold(proto));
    // DR
    aligned_write_int8(w, item_proto_get_sv_disease(proto));
    // PR
    aligned_write_int8(w, item_proto_get_sv_poison(proto));
    // hp
    aligned_write_int8(w, item_proto_get_hp(proto));
    // mana
    aligned_write_int8(w, item_proto_get_mana(proto));
    // AC
    aligned_write_int8(w, item_proto_get_ac(proto));
    // isStackable/hasUnlimitedCharges
    aligned_write_uint8(w, item_proto_is_stackable(proto) ? 1 : 0);
    // isTestItem
    aligned_write_uint8(w, 0);
    // light
    aligned_write_uint8(w, item_proto_get_light(proto));
    // delay
    aligned_write_uint8(w, item_proto_get_delay(proto));
    // damage
    aligned_write_uint8(w, item_proto_get_damage(proto));
    // clickyType
    aligned_write_uint8(w, 0); //fixme: implement this; 0 = none/proc, 1 = unrestricted clicky, 2 = worn, 3 = unrestricted expendable, 4 = must-equip clicky, 5 = class-restricted clicky
    // range
    aligned_write_uint8(w, item_proto_get_range(proto));
    // skill
    aligned_write_uint8(w, item_proto_get_item_type_id(proto));
    // isMagic
    aligned_write_uint8(w, item_proto_is_magic(proto));
    // clickableLevel
    aligned_write_uint8(w, 0); //fixme
    // material
    aligned_write_uint8(w, item_proto_get_material(proto));
    // unknownA
    aligned_write_zeroes(w, sizeof_field(Trilogy_ItemBasic, unknownA));
    // tint
    aligned_write_uint32(w, item_proto_get_tint(proto));
    // unknownB
    aligned_write_zeroes(w, sizeof_field(Trilogy_ItemBasic, unknownB));
    // spellId
    aligned_write_uint16(w, item_proto_get_spell_id(proto));
    // classesBitfield
    aligned_write_uint32(w, item_proto_get_class_bitfield(proto));
    
    // Split for basic vs bag (fixme: implement bags)
    
    // racesBitfield
    aligned_write_uint32(w, item_proto_get_race_bitfield(proto));
    // consumableType
    aligned_write_uint8(w, 3); // Weird field, is this value always good?
    
    // procLevel/hastePercent (fixme: doesn't seem to work for haste the way the 6.2 client does)
    aligned_write_uint8(w, 0);
    // charges
    aligned_write_uint8(w, item_get_charges(item));
    // effectType
    aligned_write_uint8(w, 0); //fixme: same as clickyType above
    // clickySpellId
    aligned_write_uint16(w, item_proto_get_spell_id(proto));
    // unknownC
    aligned_write_zeroes(w, sizeof_field(Trilogy_ItemBasic, unknownC));
    // castingTime
    aligned_write_uint32(w, item_proto_get_casting_time(proto));
    // unknownD
    aligned_write_zeroes(w, sizeof_field(Trilogy_ItemBasic, unknownD));
    // recommendedLevel
    aligned_write_uint8(w, 0); //fixme
    // unknownE
    aligned_write_zeroes(w, sizeof_field(Trilogy_ItemBasic, unknownE));
    // deityBitfield
    aligned_write_uint32(w, 0xffff); //fixme
    
    return packet;
}
