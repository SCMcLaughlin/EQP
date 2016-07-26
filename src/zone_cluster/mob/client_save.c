
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
    
    //fixme: do updates for the huge 'character' table...
    
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
    db_prepare_literal(db, &query, "INSERT INTO bind_point (character_id, bind_id, zone_id, x, y, z, heading) VALUES (?, ?, ?, ?, ?, ?, ?)", NULL);
    query_bind_int64(&query, 1, charId);
    
    for (i = 0; i < EQP_CLIENT_BIND_POINT_COUNT; i++)
    {
        BindPoint* b = &save->bindPoints[i];
        
        if (b->zoneId == 0)
            continue;
        
        query_bind_int(&query, 2, i);
        query_bind_int(&query, 3, b->zoneId);
        query_bind_double(&query, 4, b->loc.x);
        query_bind_double(&query, 5, b->loc.y);
        query_bind_double(&query, 6, b->loc.z);
        query_bind_double(&query, 7, b->loc.heading);
        
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

void client_save(Client* client, QueryCallback onCompletion)
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
    
    save->zc            = zc;
    save->characterId   = client_character_id(client);
    
    memcpy(save->memmedSpell, spellbook_memorized(book), sizeof_field(Spellbook, memorized));
    memcpy(&save->skill, client_skills(client), sizeof(Skills));
    memcpy(&save->bindPoints, client_bind_points(client), sizeof(BindPoint) * EQP_CLIENT_BIND_POINT_COUNT);
    
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
