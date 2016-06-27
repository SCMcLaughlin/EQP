
#include "client_packet_trilogy_output.h"
#include "zone_cluster.h"

static void client_trilogy_send_player_profile_compress_and_obfuscate(R(Client*) client, R(Trilogy_PlayerProfile*) pp)
{
    (void)client;
    (void)pp;
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
    aligned_write_float(w, client->bindPoints[0].y);
    aligned_write_float(w, client->bindPoints[1].y);
    aligned_write_float(w, client->bindPoints[2].y);
    aligned_write_float(w, client->bindPoints[3].y);
    aligned_write_float(w, client->bindPoints[4].y);
    // bindLocX[5]
    aligned_write_float(w, client->bindPoints[0].x);
    aligned_write_float(w, client->bindPoints[1].x);
    aligned_write_float(w, client->bindPoints[2].x);
    aligned_write_float(w, client->bindPoints[3].x);
    aligned_write_float(w, client->bindPoints[4].x);
    // bindLocZ[5]
    aligned_write_float(w, client->bindPoints[0].z);
    aligned_write_float(w, client->bindPoints[1].z);
    aligned_write_float(w, client->bindPoints[2].z);
    aligned_write_float(w, client->bindPoints[3].z);
    aligned_write_float(w, client->bindPoints[4].z);
    // bindLocHeading[5]
    aligned_write_float(w, client->bindPoints[0].heading);
    aligned_write_float(w, client->bindPoints[1].heading);
    aligned_write_float(w, client->bindPoints[2].heading);
    aligned_write_float(w, client->bindPoints[3].heading);
    aligned_write_float(w, client->bindPoints[4].heading);
    
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
