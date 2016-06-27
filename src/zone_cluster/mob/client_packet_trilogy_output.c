
#include "client_packet_trilogy_output.h"
#include "zone_cluster.h"

void client_trilogy_schedule_packet_individual(R(Client*) client, R(PacketTrilogy*) packet)
{
    R(ProtocolHandler*) handler = client_handler(client);
    packet_trilogy_fragmentize(packet);
    protocol_handler_trilogy_schedule_packet(&handler->trilogy, packet);
}

void client_trilogy_schedule_packet_broadcast(R(Client*) client, R(PacketTrilogy*) packet)
{
    R(ProtocolHandler*) handler = client_handler(client);
    packet_trilogy_grab(packet);
    protocol_handler_trilogy_schedule_packet(&handler->trilogy, packet);
}

static void client_trilogy_player_profile_obfuscate(R(byte*) buffer, uint32_t len)
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

static void client_trilogy_send_player_profile_compress_and_obfuscate(R(Client*) client, R(Trilogy_PlayerProfile*) pp)
{
    byte buffer[sizeof(Trilogy_PlayerProfile)];
    unsigned long length    = sizeof(buffer);
    R(ZC*) zc               = client_zone_cluster(client);
    R(PacketTrilogy*) packet;
    int rc;
    
    rc = compress2(buffer, &length, (const byte*)pp, sizeof(Trilogy_PlayerProfile), Z_BEST_COMPRESSION);
    
    if (rc != Z_OK)
        exception_throw_format(B(zc), ErrorCompression, "[client_trilogy_send_player_profile_compress_and_obfuscate] ZLib compression failed, err code %i", rc);
    
    client_trilogy_player_profile_obfuscate(buffer, length);
    
    packet = packet_trilogy_create(B(zc), TrilogyOp_PlayerProfile, length);
    memcpy(packet_trilogy_data(packet), buffer, length);
    
    client_trilogy_schedule_packet_individual(client, packet);
}

void client_trilogy_send_player_profile(R(Client*) client)
{
    Trilogy_PlayerProfile pp;
    Aligned write;
    R(Aligned*) w = &write;
    InventoryIterator invItr;
    R(Inventory*) inv   = &client->inventory;
    uint64_t time       = clock_milliseconds();
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
        R(InventorySlot*) slot = invItr.slot;
        
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
        R(InventorySlot*) slot = invItr.slot;
        
        aligned_advance(w, sizeof(Trilogy_PPItem) * (slot->slotId - 1)); // No charm slot
        
        aligned_advance(w, sizeof(uint16_t));
        aligned_write_uint8(w, slot->charges);
        
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
        R(InventorySlot*) slot = invItr.slot;
        
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
        R(InventorySlot*) slot = invItr.slot;
        
        aligned_advance(w, sizeof(Trilogy_PPItem) * (slot->slotId - InvSlot_BagsSlotsIncludingCursorBegin));
        
        aligned_advance(w, sizeof(uint16_t));
        aligned_write_uint8(w, slot->charges);
        
        aligned_reset_to(w, reset);
    }
    
    aligned_advance(w, sizeof(pp.baggedItemProperties));
    
    // spellbook
    aligned_write_memset_no_advance(w, 0xff, sizeof(pp.spellbook));
    
    if (!client_spellbook_is_empty(client))
    {
        R(SpellbookSlot*) array = array_data_type(client->spellbook.knownSpells, SpellbookSlot);
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
    aligned_advance(w, 96);
    
    // autoSplit
    aligned_write_uint32(w, client->isAutoSplitEnabled ? 1 : 0);
    // pvpEnabled
    aligned_write_uint32(w, client->isPvP ? 1 : 0);
    // unknownG[12]
    aligned_advance(w, sizeof(pp.unknownG));
    // isGM
    aligned_write_uint32(w, client->isGM ? 1 : 0);
    // unknownH[20]
    aligned_advance(w, sizeof(pp.unknownH));
    // disciplinesReady
    aligned_write_uint32(w, (time >= client->disciplineTimestamp));
    // unknownI[20]
    aligned_advance(w, 20);
    // hunger
    aligned_write_uint32(w, client->hungerLevel);
    // thirst
    aligned_write_uint32(w, client->thirstLevel);
    // unknownJ[24]
    aligned_advance(w, 24);
    
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
        R(InventorySlot*) slot = invItr.slot;
        
        aligned_advance(w, sizeof(Trilogy_PPItem) * (slot->slotId - InvSlot_BankBegin));
        
        aligned_advance(w, sizeof(uint16_t));
        aligned_write_uint8(w, slot->charges);

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
        R(InventorySlot*) slot = invItr.slot;
        
        aligned_advance(w, sizeof(Trilogy_PPItem) * (slot->slotId - InvSlot_BankBagSlotsBegin));
        
        aligned_advance(w, sizeof(uint16_t));
        aligned_write_uint8(w, slot->charges);
        
        aligned_reset_to(w, reset);
    }
    
    aligned_advance(w, sizeof(pp.bankBaggedItemProperties));
    
    // unknownK
    aligned_advance(w, sizeof(uint32_t));
    
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
        R(InventorySlot*) slot = invItr.slot;
        
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
        R(InventorySlot*) slot = invItr.slot;
        
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
    aligned_write_uint32(w, time);
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
        // name
        aligned_advance(w, sizeof_field(Trilogy_PPGroupMember, name));
        // unknownADefaultFFFFFFFF
        aligned_write_uint32(w, 0xffffffff);
        // unknownB[2]
        aligned_advance(w, sizeof_field(Trilogy_PPGroupMember, unknownB));
        // unknownCDefaultFFFFFFFF
        aligned_write_uint32(w, 0xffffffff);
        // unknownD
        aligned_advance(w, sizeof_field(Trilogy_PPGroupMember, unknownD));
        // unknownEDefaultFFFFFFFF
        aligned_write_uint32(w, 0xffffffff);
        // unknownF
        aligned_advance(w, sizeof_field(Trilogy_PPGroupMember, unknownF));
    }
    
    // unknownW
    aligned_advance(w, 30);
    aligned_write_uint16(w, 0xffff);
    aligned_advance(w, 6);
    aligned_write_memset(w, 0xff, 6);
    aligned_advance(w, 176);
    aligned_write_uint16(w, 0xffff);
    aligned_advance(w, 2);
    
    // unknownX
    for (i = 0; i < 60; i++)
    {
        // name
        aligned_advance(w, sizeof_field(Trilogy_PPGroupMember, name));
        // unknownADefaultFFFFFFFF
        aligned_write_uint32(w, 0xffffffff);
        // unknownB[2]
        aligned_advance(w, sizeof_field(Trilogy_PPGroupMember, unknownB));
        // unknownCDefaultFFFFFFFF
        aligned_write_uint32(w, 0xffffffff);
        // unknownD
        aligned_advance(w, sizeof_field(Trilogy_PPGroupMember, unknownD));
        // unknownEDefaultFFFFFFFF
        aligned_write_uint32(w, 0xffffffff);
        // unknownF
        aligned_advance(w, sizeof_field(Trilogy_PPGroupMember, unknownF));
    }
    
    // unknownY[20] is already zeroed, don't need to bother with it

    // crc
    pp.crc = ~crc_calc32(((byte*)&pp) + sizeof(uint32_t), sizeof(pp) - sizeof(uint32_t));
    
    client_trilogy_send_player_profile_compress_and_obfuscate(client, &pp);
}

void client_trilogy_send_zone_entry(R(Client*) client)
{
    R(PacketTrilogy*) packet;
    Aligned write;
    R(Aligned*) w   = &write;
    R(ZC*) zc       = client_zone_cluster(client);
    
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
    aligned_write_uint8(w, client->isPvP ? 1 : 0);
    // unknownF
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownF));
    // face
    aligned_write_uint8(w, client_face(client));
    // helmMaterial
    aligned_write_uint8(w, 0); //fixme: handle this
    // unknownG
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownG));
    // secondaryWeaponModelId
    aligned_write_uint8(w, 0); //fixme: handle this
    // primaryWeaponModelId
    aligned_write_uint8(w, 0); //fixme: handle this
    // unknownH
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownH));
    // helmColor
    aligned_write_uint32(w, 0); //fixme: handle this
    // unknownI
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownI));
    // texture
    aligned_write_uint8(w, 0xff);
    // unknownJ
    aligned_advance(w, sizeof_field(Trilogy_ZoneEntry, unknownJ));
    // walkingSpeed
    aligned_write_float(w, 0.46f); //fixme: handle this
    // runningSpeed
    aligned_write_float(w, 0.7f); //fixme: handle this
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

PacketTrilogy* client_trilogy_make_op_weather(R(ZC*) zc, int weatherType, int intensity)
{
    R(PacketTrilogy*) packet = packet_trilogy_create(B(zc), TrilogyOp_Weather, sizeof(Trilogy_Weather));
    Aligned w;
    
    aligned_init(B(zc), &w, packet_trilogy_data(packet), packet_trilogy_length(packet));
    // type
    aligned_write_int(&w, weatherType);
    // intensity
    aligned_write_int(&w, intensity);
    
    return packet;
}

PacketTrilogy* client_trilogy_make_op_zone_info(R(ZC*) zc, R(Zone*) zone)
{
    R(PacketTrilogy*) packet = packet_trilogy_create(B(zc), TrilogyOp_ZoneInfo, sizeof(Trilogy_ZoneInfo));
    Aligned write;
    R(Aligned*) w = &write;
    
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
