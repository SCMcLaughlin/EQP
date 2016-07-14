
#include "item_gen.h"

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

static void item_gen_write_to_disk(ItemGen* gen)
{
    ItemShm header;
    ItemDbEntry* dbEntries      = array_data_type(gen->itemPrototypes, ItemPrototype);
    uint32_t n                  = array_count(gen->itemPrototypes);
    uint32_t cap                = bit_pow2_greater_or_equal(n);
    ItemShm_Entry* hashEntries  = eqp_alloc_type_array(B(gen), cap, ItemShm_Entry);
    FILE* fp;
    uint32_t i;
    
    header.timestamp = clock_unix_seconds();
    
    for (i = 0; i < cap; i++)
    {
        ItemShm_Entry* ent = &hashEntries[i];
        
        ent->key    = 0;
        ent->hash   = 0;
        ent->next   = 0xffffffff;
        ent->offset = 0;
    }
    
    cap--;
    
    for (i = 0; i < n; i++)
    {
        ItemDbEntry* dbEnt  = &dbEntries[i];
        uint32_t hash       = item_share_mem_calc_hash(dbEnt->itemId);
        //do insert
    }
    
    header.capacityMinusOne = cap;
    
    fp = fopen("fixme", "wb+");
    
    if (!fp)
        return; //exception_throw
    
    // Header
    fwrite(&header, 1, sizeof(header), fp);
    // Hash table
    fwrite(hashEntries, sizeof(ItemShm_Entry), cap + 1, fp);
    // ItemPrototypes
    fwrite(dbEntries, sizeof(ItemPrototype), n, fp);
    
    fclose(fp);
}

void item_gen_scan_and_generate(ItemGen* gen)
{
    item_gen_read_db_entries(gen);
    item_gen_run_lua_script(gen);
    item_gen_commit_changes(gen);
    item_gen_write_to_disk(gen);
}

void item_gen_add(ItemGen* gen, ItemPrototype* proto, const char* scriptPath, uint32_t len, int changed)
{
    ItemDbEntry insert;
    ItemDbEntry* ent = hash_table_get_type_by_cstr(gen->scriptPathToDbEntry, scriptPath, len, ItemDbEntry);
    
    if (!ent)
    {
        len++; // Include null terminator
        
        insert.itemId   = item_proto_get_item_id(proto);
        insert.isUpdate = false;
        insert.addPath  = eqp_alloc_type_bytes(B(gen), len, char);
        
        memcpy(insert.addPath, scriptPath, len);
        
        ent     = &insert;
        changed = true;
    }
    
    //fixme: make sure scriptPath is set in the prototype before this call
    array_push_back(B(gen), &gen->itemPrototypes, proto);
    
    if (changed)
        array_push_back(B(gen), &gen->changes, ent);
}
