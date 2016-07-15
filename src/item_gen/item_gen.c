
#include "item_gen.h"

#define INVALID 0xffffffff

void item_gen_init(ItemGen* gen)
{
    core_init(C(gen), 0, NULL);
    
    gen->nextItemId             = 1000;
    gen->itemPrototypes         = array_create_type(B(gen), ItemPrototype);
    gen->scriptPathToDbEntry    = hash_table_create_type(B(gen), ItemDbEntry);
    gen->changes                = array_create_type(B(gen), ItemDbEntry);
    gen->L                      = lua_sys_open(B(gen));
}

void item_gen_deinit(ItemGen* gen)
{
    if (gen->itemPrototypes)
    {
        array_destroy(gen->itemPrototypes);
        gen->itemPrototypes = NULL;
    }
    
    if (gen->scriptPathToDbEntry)
    {
        hash_table_destroy(gen->scriptPathToDbEntry);
        gen->scriptPathToDbEntry = NULL;
    }
    
    if (gen->changes)
    {
        ItemDbEntry* entries    = array_data_type(gen->changes, ItemDbEntry);
        uint32_t n              = array_count(gen->changes);
        uint32_t i;
        
        for (i = 0; i < n; i++)
        {
            ItemDbEntry* ent = &entries[i];
            
            if (ent->addPath)
                free(ent->addPath);
        }
        
        array_destroy(gen->changes);
        gen->changes = NULL;
    }
    
    if (gen->L)
    {
        lua_close(gen->L);
        gen->L = NULL;
    }
    
    core_deinit(C(gen));
}

static void item_gen_read_db_entries(ItemGen* gen)
{
    Database* db = core_db(C(gen));
    Query query;
    
    query_init(&query);
    db_prepare_literal(db, &query, "SELECT MAX(item_id) FROM item", NULL);
    query_execute_synchronus(&query);
    
    while (query_select(&query))
    {
        if (!query_is_null(&query, 1))
            gen->nextItemId = query_get_int64(&query, 1) + 1;
    }
    
    query_deinit(&query);
    
    query_init(&query);
    db_prepare_literal(db, &query, "SELECT item_id, timestamp, script_path FROM item", NULL);
    query_execute_synchronus(&query);
    
    while (query_select(&query))
    {
        ItemDbEntry ent;
        const char* path;
        uint32_t len;
        
        ent.itemId      = (uint32_t)query_get_int64(&query, 1);
        ent.isUpdate    = true;
        ent.timestamp   = query_get_int64(&query, 2);
        ent.addPath     = NULL;
        path            = query_get_string(&query, 3, &len);
        
        hash_table_set_by_cstr(B(gen), &gen->scriptPathToDbEntry, path, len, &ent);
    }
    
    query_deinit(&query);
}

static void item_gen_run_lua_script(ItemGen* gen)
{
    lua_State* L = gen->L;
    
    lua_pushlightuserdata(L, gen);
    lua_setglobal(L, "itemGen");
    
    lua_sys_run_file(B(gen), L, EQP_ITEM_GEN_SCRIPT, 0);
}

static void item_gen_commit_changes(ItemGen* gen)
{
    Database* db            = core_db(C(gen));
    ItemDbEntry* entries    = array_data_type(gen->changes, ItemDbEntry);
    uint32_t n              = array_count(gen->changes);
    uint64_t timestamp      = clock_unix_seconds();
    Query query;
    Query update;
    Query insert;
    uint32_t i;
    
    query_init(&query);
    db_prepare_literal(db, &query, "BEGIN", NULL);
    query_execute_synchronus(&query);
    query_deinit(&query);
    
    query_init(&update);
    db_prepare_literal(db, &update, "UPDATE item SET timestamp = ? WHERE item_id = ?", NULL);
    query_bind_int64(&update, 1, timestamp);
    
    query_init(&insert);
    db_prepare_literal(db, &insert, "INSERT INTO item (item_id, timestamp, script_path) VALUES (?, ?, ?)", NULL);
    query_bind_int64(&insert, 2, timestamp);
    
    for (i = 0; i < n; i++)
    {
        ItemDbEntry* ent = &entries[i];
        
        if (ent->isUpdate)
        {
            query_bind_int64(&update, 2, ent->itemId);
            query_execute_synchronus_insert_update(&update);
        }
        else
        {
            query_bind_int64(&insert, 1, ent->itemId);
            query_bind_string_no_copy(&insert, 3, ent->addPath, QUERY_CALC_LENGTH);
            query_execute_synchronus_insert_update(&insert);
        }
    }
    
    query_deinit(&update);
    query_deinit(&insert);
    
    query_init(&query);
    db_prepare_literal(db, &query, "COMMIT", NULL);
    query_execute_synchronus(&query);
    query_deinit(&query);
}

static void item_gen_advance_free_index(ItemShm_Entry* entries, uint32_t capMinusOne, uint32_t* freeIndex)
{
    uint32_t i;
    
    for (i = 0; i <= capMinusOne; i++)
    {
        if (entries[i].offset == 0)
        {
            *freeIndex = i;
            return;
        }
    }
}

static void item_gen_set_hash_entry(ItemShm_Entry* ent, uint32_t itemId, uint32_t hash, uint32_t* offset)
{
    ent->key    = itemId;
    ent->hash   = hash;
    ent->offset = *offset;
    *offset += sizeof(ItemPrototype);
}

static void item_gen_write_to_disk(ItemGen* gen)
{
    ItemShm header;
    ItemPrototype* prototypes   = array_data_type(gen->itemPrototypes, ItemPrototype);
    uint32_t n                  = array_count(gen->itemPrototypes);
    uint32_t cap                = bit_pow2_greater_or_equal(n);
    ItemShm_Entry* hashEntries  = eqp_alloc_type_array(B(gen), cap, ItemShm_Entry);
    uint32_t freeIndex          = 0;
    uint32_t offset             = sizeof(header) + (sizeof(ItemShm_Entry) * cap);
    FILE* fp;
    uint32_t i;
    
    header.itemCount = n;
    header.timestamp = clock_unix_seconds();
    
    for (i = 0; i < cap; i++)
    {
        ItemShm_Entry* ent = &hashEntries[i];
        
        ent->key    = 0;
        ent->hash   = 0;
        ent->next   = INVALID;
        ent->offset = 0;
    }
    
    cap--;
    
    for (i = 0; i < n; i++)
    {
        ItemPrototype* proto    = &prototypes[i];
        uint32_t itemId         = item_proto_get_item_id(proto);
        uint32_t hash           = item_share_mem_calc_hash(itemId);
        uint32_t index          = hash & cap;
        ItemShm_Entry* ent      = &hashEntries[index];
        
        if (ent->offset == 0)
        {
            item_gen_set_hash_entry(ent, itemId, hash, &offset);
            
            if (index == freeIndex)
                item_gen_advance_free_index(hashEntries, cap, &freeIndex);
        }
        else
        {
            uint32_t mainIndex = ent->hash & cap;
            
            if (mainIndex != index)
            {
                ItemShm_Entry* mainEnt;
                
                // Evict this entry to the free index
                hashEntries[freeIndex] = *ent;
                
                // Follow this entry's chain to the end, then point the last entry to the free index
                mainEnt = &hashEntries[mainIndex];
                
                while (mainEnt->next != index)
                {
                    mainEnt = &hashEntries[mainEnt->next];
                }
                
                mainEnt->next   = freeIndex;
                ent->next       = INVALID;
                item_gen_set_hash_entry(ent, itemId, hash, &offset);
                item_gen_advance_free_index(hashEntries, cap, &freeIndex);
            }
            else
            {
                for (;;)
                {
                    if (ent->key == itemId)
                        exception_throw_format(B(gen), ErrorDuplicate, "Duplicate itemId '%u' found, aborting", itemId);
                    
                    if (ent->next != INVALID)
                        ent = &hashEntries[ent->next];
                    else
                        break;
                }

                ent->next = freeIndex;
                item_gen_set_hash_entry(&hashEntries[freeIndex], itemId, hash, &offset);
                item_gen_advance_free_index(hashEntries, cap, &freeIndex);
            }
        }
    }
    
    header.capacityMinusOne = cap;
    
    fp = fopen(EQP_ITEM_GEN_SHM_PATH_DEFAULT, "wb+");
    
    if (!fp)
        exception_throw_format(B(gen), ErrorDoesNotExist, "Could not open '%s' for writing", EQP_ITEM_GEN_SHM_PATH_DEFAULT);
    
    // Header
    fwrite(&header, 1, sizeof(header), fp);
    // Hash table
    fwrite(hashEntries, sizeof(ItemShm_Entry), cap + 1, fp);
    // ItemPrototypes
    fwrite(prototypes, sizeof(ItemPrototype), n, fp);
    
    fclose(fp);
}

void item_gen_scan_and_generate(ItemGen* gen)
{
    item_gen_read_db_entries(gen);
    item_gen_run_lua_script(gen);
    item_gen_commit_changes(gen);
    item_gen_write_to_disk(gen);
}

void item_gen_add(ItemGen* gen, ItemPrototype* proto, const char* scriptPath, uint32_t len, uint64_t timestamp)
{
    ItemDbEntry insert;
    ItemDbEntry* ent    = hash_table_get_type_by_cstr(gen->scriptPathToDbEntry, scriptPath, len, ItemDbEntry);
    int changed         = false;
    
    if (!ent)
    {
        len++; // Include null terminator
        
        item_proto_set_item_id(proto, gen->nextItemId++);
        
        insert.itemId   = item_proto_get_item_id(proto);
        insert.isUpdate = false;
        insert.addPath  = eqp_alloc_type_bytes(B(gen), len, char);
        
        memcpy(insert.addPath, scriptPath, len);
        
        ent     = &insert;
        changed = true;
    }
    else
    {
        item_proto_set_item_id(proto, ent->itemId);
        
        if (timestamp > ent->timestamp)
            changed = true;
    }
    
    item_proto_set_script_path(proto, scriptPath);
    
    array_push_back(B(gen), &gen->itemPrototypes, proto);
    
    if (changed)
        array_push_back(B(gen), &gen->changes, ent);
}

#undef INVALID
