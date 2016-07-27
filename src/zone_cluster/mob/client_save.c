
#include "client_save.h"
#include "client.h"
#include "zone_cluster.h"

static void client_save_do_delete(Database* db, const char* str, int len, int64_t charId)
{
    Query query;
    
    query_init(&query);
    db_prepare(db, &query, str, len, NULL);
    query_bind_int64(&query, 1, charId);
    query_execute_synchronus(&query);
    query_deinit(&query);
}

#define client_save_delete_literal(db, str, charId) client_save_do_delete((db), (str), sizeof(str) - 1, (charId))

static void client_save_transaction(Transaction* trans)
{
    ClientSave* save    = transaction_userdata_type(trans, ClientSave);
    Database* db        = transaction_db(trans);
    int64_t charId      = save->characterId;
    byte* ptr           = ((byte*)save) + sizeof(ClientSave);
    Query query;
    uint32_t n;
    uint32_t i;
    
    query_init(&query);
    db_prepare_literal(db, &query,
        "UPDATE character SET "
            "name = ?, surname = ?, level = ?, class = ?, race = ?, zone_id = ?, instance_id = ?, gender = ?, face = ?, deity = ?, "
            "x = ?, y = ?, z = ?, heading = ?, current_hp = ?, current_mana = ?, current_endurance = ?, experience = ?, training_points = ?, "
            "base_str = ?, base_sta = ?, base_dex = ?, base_agi = ?, base_int = ?, base_wis = ?, base_cha = ?, guild_id = ?, guild_rank = ?, "
            "harmtouch_timestamp = ?, discipline_timestamp = ?, pp = ?, gp = ?, sp = ?, cp = ?, pp_cursor = ?, gp_cursor = ?, sp_cursor = ?, "
            "cp_cursor = ?, pp_bank = ?, gp_bank = ?, sp_bank = ?, cp_bank = ?, hunger = ?, thirst = ?, is_gm = ?, anon = ?, drunkeness = ?, "
            "material0 = ?, material1 = ?, material2 = ?, material3 = ?, material4 = ?, material5 = ?, material6 = ?, material7 = ?, material8 = ?, "
            "tint0 = ?, tint1 = ?, tint2 = ?, tint3 = ?, tint4 = ?, tint5 = ?, tint6 = ? "
        "WHERE character_id = ?",
        NULL);
    
    query_bind_string(&query, 1, save->name, QUERY_CALC_LENGTH);
    query_bind_string(&query, 2, save->surname, QUERY_CALC_LENGTH);
    query_bind_int(&query, 3, save->level);
    query_bind_int(&query, 4, save->class);
    query_bind_int(&query, 5, save->race);
    query_bind_int(&query, 6, save->point.zoneId);
    query_bind_int(&query, 7, save->point.instanceId);
    query_bind_int(&query, 8, save->gender);
    query_bind_int(&query, 9, save->face);
    query_bind_int(&query, 10, save->deity);
    query_bind_double(&query, 11, save->point.loc.x);
    query_bind_double(&query, 12, save->point.loc.y);
    query_bind_double(&query, 13, save->point.loc.z);
    query_bind_double(&query, 14, save->point.loc.heading);
    query_bind_int(&query, 15, save->hp);
    query_bind_int(&query, 16, save->mana);
    query_bind_int(&query, 17, save->endurance);
    query_bind_int64(&query, 18, save->experience);
    query_bind_int(&query, 19, save->trainingPoints);
    query_bind_int(&query, 20, save->STR);
    query_bind_int(&query, 21, save->STA);
    query_bind_int(&query, 22, save->DEX);
    query_bind_int(&query, 23, save->AGI);
    query_bind_int(&query, 24, save->INT);
    query_bind_int(&query, 25, save->WIS);
    query_bind_int(&query, 26, save->CHA);
    query_bind_int(&query, 27, save->guildId);
    query_bind_int(&query, 28, save->guildRank);
    query_bind_int64(&query, 29, save->harmtouchTimestamp);
    query_bind_int64(&query, 30, save->disciplineTimestamp);
    query_bind_int(&query, 31, save->coins.pp);
    query_bind_int(&query, 32, save->coins.gp);
    query_bind_int(&query, 33, save->coins.sp);
    query_bind_int(&query, 34, save->coins.cp);
    query_bind_int(&query, 35, save->coinsCursor.pp);
    query_bind_int(&query, 36, save->coinsCursor.gp);
    query_bind_int(&query, 37, save->coinsCursor.sp);
    query_bind_int(&query, 38, save->coinsCursor.cp);
    query_bind_int(&query, 39, save->coinsBank.pp);
    query_bind_int(&query, 40, save->coinsBank.gp);
    query_bind_int(&query, 41, save->coinsBank.sp);
    query_bind_int(&query, 42, save->coinsBank.cp);
    query_bind_int(&query, 43, save->hungerLevel);
    query_bind_int(&query, 44, save->thirstLevel);
    query_bind_int(&query, 45, save->isGM);
    query_bind_int(&query, 46, save->anonSetting);
    query_bind_int(&query, 47, save->drunkeness);
    query_bind_int(&query, 48, save->material[0]);
    query_bind_int(&query, 49, save->material[1]);
    query_bind_int(&query, 50, save->material[2]);
    query_bind_int(&query, 51, save->material[3]);
    query_bind_int(&query, 52, save->material[4]);
    query_bind_int(&query, 53, save->material[5]);
    query_bind_int(&query, 54, save->material[6]);
    query_bind_int(&query, 55, save->primaryModelId);
    query_bind_int(&query, 56, save->secondaryModelId);
    query_bind_int(&query, 57, save->tint[0]);
    query_bind_int(&query, 58, save->tint[1]);
    query_bind_int(&query, 59, save->tint[2]);
    query_bind_int(&query, 60, save->tint[3]);
    query_bind_int(&query, 61, save->tint[4]);
    query_bind_int(&query, 62, save->tint[5]);
    query_bind_int(&query, 63, save->tint[6]);
    
    query_bind_int64(&query, 64, charId);

    query_execute_synchronus_insert_update(&query);
    query_deinit(&query);
    
    ////////////////////////////////////////////////////////////////////////////////
    // Memmed Spells
    ////////////////////////////////////////////////////////////////////////////////
    client_save_delete_literal(db, "DELETE FROM memmed_spells WHERE character_id = ?", charId);
    
    query_init(&query);
    db_prepare_literal(db, &query, "INSERT INTO memmed_spells (character_id, slot_id, spell_id, recast_timestamp_milliseconds) VALUES (?, ?, ?, ?)", NULL);
    query_bind_int64(&query, 1, charId);
    
    for (i = 0; i < EQP_MEMORIZED_SPELL_SLOTS; i++)
    {
        MemorizedSpell* mem = &save->memmedSpell[i];
        
        if (mem->spellId == 0)
            continue;
        
        query_bind_int(&query, 2, i);
        query_bind_int(&query, 3, mem->spellId);
        query_bind_int64(&query, 4, mem->recastTimestamp);
        
        query_execute_synchronus_insert_update(&query);
    }
    
    query_deinit(&query);
    
    ////////////////////////////////////////////////////////////////////////////////
    // Skills and Languages
    ////////////////////////////////////////////////////////////////////////////////
    client_save_delete_literal(db, "DELETE FROM skill WHERE character_id = ?", charId);
    
    query_init(&query);
    db_prepare_literal(db, &query, "INSERT INTO skill (character_id, skill_id, value) VALUES (?, ?, ?)", NULL);
    query_bind_int64(&query, 1, charId);
    
    for (i = 0; i < Skill_COUNT; i++)
    {
        uint8_t value = save->skill.skill[i];
        
        if (value == 0 || value > EQP_SKILL_MAX_VALUE)
            continue;
        
        query_bind_int(&query, 2, i);
        query_bind_int(&query, 3, value);
        
        query_execute_synchronus_insert_update(&query);
    }
    
    for (i = 0; i < Lang_COUNT; i++)
    {
        uint8_t value = save->skill.language[i];
        
        if (value == 0 || value > EQP_SKILL_MAX_VALUE)
            continue;
        
        query_bind_int(&query, 2, i + EQP_DB_LANGUAGE_SKILL_OFFSET);
        query_bind_int(&query, 3, value);
        
        query_execute_synchronus_insert_update(&query);
    }
    
    query_deinit(&query);
    
    ////////////////////////////////////////////////////////////////////////////////
    // Bind Points
    ////////////////////////////////////////////////////////////////////////////////
    client_save_delete_literal(db, "DELETE FROM bind_point WHERE character_id = ?", charId);
    
    query_init(&query);
    db_prepare_literal(db, &query, "INSERT INTO bind_point (character_id, bind_id, zone_id, instance_id, x, y, z, heading) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", NULL);
    query_bind_int64(&query, 1, charId);
    
    for (i = 0; i < EQP_CLIENT_BIND_POINT_COUNT; i++)
    {
        BindPoint* b = &save->bindPoints[i];
        
        if (b->zoneId == 0)
            continue;
        
        query_bind_int(&query, 2, i);
        query_bind_int(&query, 3, b->zoneId);
        query_bind_int(&query, 4, b->instanceId);
        query_bind_double(&query, 5, b->loc.x);
        query_bind_double(&query, 6, b->loc.y);
        query_bind_double(&query, 7, b->loc.z);
        query_bind_double(&query, 8, b->loc.heading);
        
        query_execute_synchronus_insert_update(&query);
    }
    
    query_deinit(&query);
    
    ////////////////////////////////////////////////////////////////////////////////
    // Inventory
    ////////////////////////////////////////////////////////////////////////////////
    client_save_delete_literal(db, "DELETE FROM inventory WHERE character_id = ?", charId);
    
    n = save->invCount;
    if (n > 0)
    {
        ClientSave_Inv* inv = (ClientSave_Inv*)ptr;
        
        ptr += sizeof(ClientSave_Inv) * n;
        
        query_init(&query);
        db_prepare_literal(db, &query, "INSERT INTO inventory (character_id, slot_id, aug_slot_id, stack_amount, charges, item_id) VALUES (?, ?, ?, ?, ?, ?)", NULL);
        
        query_bind_int64(&query, 1, charId);
        
        for (i = 0; i < n; i++)
        {
            ClientSave_Inv* slot = &inv[i];
            
            if (slot->itemId == 0)
                continue;
            
            query_bind_int(&query, 2, slot->slotId);
            query_bind_int(&query, 3, slot->augSlotId);
            query_bind_int(&query, 4, slot->stackAmount);
            query_bind_int(&query, 5, slot->charges);
            query_bind_int64(&query, 6, slot->itemId);
            
            query_execute_synchronus_insert_update(&query);
        }
        
        query_deinit(&query);
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // Spellbook
    ////////////////////////////////////////////////////////////////////////////////
    client_save_delete_literal(db, "DELETE FROM spellbook WHERE character_id = ?", charId);
    
    n = save->spellbookCount;
    if (n > 0)
    {
        SpellbookSlot* spells = (SpellbookSlot*)ptr;
        
        ptr += sizeof(SpellbookSlot) * n;
        
        query_init(&query);
        db_prepare_literal(db, &query, "INSERT INTO spellbook (character_id, slot_id, spell_id) VALUES (?, ?, ?)", NULL);
        
        query_bind_int64(&query, 1, charId);
        
        for (i = 0; i < n; i++)
        {
            SpellbookSlot* spell = &spells[i];
            
            query_bind_int(&query, 2, spell->slotId);
            query_bind_int(&query, 3, spell->spellId);
            
            query_execute_synchronus_insert_update(&query);
        }
        
        query_deinit(&query);
    }
    
    // If we have no onCompletion callback, free the client snapshot.
    // If we do have a callback, it's the callback's responsibility to free the snapshot;
    // presumably it will want to have it available to figure out which client just finished saving.
    if (!transaction_query_callback(trans))
        free(save);
}

void client_save_override_location_and_vitals(Client* client, QueryCallback onCompletion, int isAutoSave, BindPoint* point, int hp, int mana, int endurance)
{
    ZC* zc              = client_zone_cluster(client);
    Inventory* inv      = client_inventory(client);
    Spellbook* book     = client_spellbook(client);
    Database* db        = core_db(C(zc));
    uint32_t invCount   = inventory_array_count(inv);
    uint32_t bookCount  = spellbook_is_empty(book) ? 0 : spellbook_array_count(book);
    uint32_t length     = sizeof(ClientSave) + (sizeof(ClientSave_Inv) * invCount) + (sizeof(SpellbookSlot) * bookCount);
    byte* ptr           = eqp_alloc_type_bytes(B(zc), length, byte);
    ClientSave* save    = (ClientSave*)ptr;
    uint32_t i;
    
    ptr += sizeof(ClientSave);
    
    // If this is an on-demand save, we can put off the next auto-save
    if (!isAutoSave)
        timer_restart(&client->timerAutoSave);
    
    save->zc                    = zc;
    save->characterId           = client_character_id(client);
    snprintf(save->name, sizeof_field(ClientSave, name), "%s", client_name_cstr(client));
    snprintf(save->surname, sizeof_field(ClientSave, surname), "%s", client_surname_cstr(client));
    save->level                 = client_level(client);
    save->class                 = client_class(client);
    save->gender                = client_base_gender(client);
    save->face                  = client_face(client);
    save->race                  = client_base_race(client);
    save->deity                 = client_deity(client);
    memcpy(&save->point, point, sizeof(BindPoint));
    save->hp                    = hp;
    save->mana                  = mana;
    save->endurance             = endurance;
    save->STR                   = client_base_str(client);
    save->DEX                   = client_base_dex(client);
    save->AGI                   = client_base_agi(client);
    save->STA                   = client_base_sta(client);
    save->INT                   = client_base_int(client);
    save->WIS                   = client_base_wis(client);
    save->CHA                   = client_base_cha(client);
    memcpy(&save->coins, client_coins(client), sizeof(Coin));
    memcpy(&save->coinsBank, client_coins_bank(client), sizeof(Coin));
    memcpy(&save->coinsCursor, client_coins_cursor(client), sizeof(Coin));
    save->guildRank             = client_guild_rank(client);
    save->guildId               = client_guild_id(client);
    save->harmtouchTimestamp    = client_harmtouch_timestamp(client);
    save->hungerLevel           = client_hunger_level(client);
    save->thirstLevel           = client_thirst_level(client);
    save->trainingPoints        = client_training_points(client);
    save->experience            = client_experience(client);
    save->anonSetting           = client_anon_setting(client);
    save->isGM                  = client_is_gm(client);
    save->drunkeness            = client_drunkeness(client);
    save->material[0]           = client_get_material(client, 0);
    save->material[1]           = client_get_material(client, 1);
    save->material[2]           = client_get_material(client, 2);
    save->material[3]           = client_get_material(client, 3);
    save->material[4]           = client_get_material(client, 4);
    save->material[5]           = client_get_material(client, 5);
    save->material[6]           = client_get_material(client, 6);
    save->tint[0]               = client_get_tint(client, 0);
    save->tint[1]               = client_get_tint(client, 1);
    save->tint[2]               = client_get_tint(client, 2);
    save->tint[3]               = client_get_tint(client, 3);
    save->tint[4]               = client_get_tint(client, 4);
    save->tint[5]               = client_get_tint(client, 5);
    save->tint[6]               = client_get_tint(client, 6);
    save->primaryModelId        = client_primary_model_id(client);
    save->secondaryModelId      = client_secondary_model_id(client);
    
    memcpy(save->memmedSpell, spellbook_memorized(book), sizeof_field(Spellbook, memorized));
    memcpy(&save->skill, client_skills(client), sizeof(Skills));
    memcpy(save->bindPoints, client_bind_points(client), sizeof(BindPoint) * EQP_CLIENT_BIND_POINT_COUNT);
    
    save->invCount          = invCount;
    save->spellbookCount    = bookCount;
    
    if (invCount > 0)
    {
        InventorySlot* slots = inventory_array(inv);
        
        for (i = 0; i < invCount; i++)
        {
            InventorySlot* src  = &slots[i];
            ClientSave_Inv* dst = (ClientSave_Inv*)ptr;
            
            ptr += sizeof(ClientSave_Inv);
            
            dst->slotId     = src->slotId;
            dst->augSlotId  = src->augSlotId;
            dst->itemId     = src->itemId;
            
            if (src->item)
            {
                dst->stackAmount    = item_get_stack_amount(src->item);
                dst->charges        = item_get_charges(src->item);
            }
        }
    }
    
    if (bookCount > 0)
        memcpy(ptr, spellbook_array(book), sizeof(SpellbookSlot) * bookCount);
    
    db_schedule_transaction(db, save, client_save_transaction, onCompletion);
}

void client_save(Client* client, QueryCallback onCompletion, int isAutoSave)
{
    Zone* zone = client_zone(client);
    BindPoint point;
    
    point.zoneId        = zone_get_zone_id(zone);
    point.instanceId    = zone_get_instance_id(zone);
    point.loc.x         = client_x(client);
    point.loc.y         = client_y(client);
    point.loc.z         = client_z(client);
    point.loc.heading   = client_heading(client);
    
    client_save_override_location_and_vitals(client, onCompletion, isAutoSave, &point,
        client_current_hp(client), client_current_mana(client), client_current_endurance(client));
}
