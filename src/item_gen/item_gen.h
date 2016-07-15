
#ifndef EQP_ITEM_GEN_H
#define EQP_ITEM_GEN_H

#include "define.h"
#include "eqp_core.h"
#include "eqp_clock.h"
#include "eqp_alloc.h"
#include "eqp_array.h"
#include "eqp_hash_table.h"
#include "item_prototype.h"
#include "item_share_mem.h"
#include "database.h"
#include "query.h"
#include "lua_sys.h"

#define EQP_ITEM_GEN_SCRIPT             "scripts/item_gen/item_gen_main.lua"
#define EQP_ITEM_GEN_SHM_PATH_DEFAULT   "shm/eqp_item_prototypes"
#define EQP_ITEM_GEN_SHM_PATH_ALTERNATE "shm/eqp_item_prototypes2"

STRUCT_DEFINE(ItemGen)
{
    Core        core;
    int64_t     nextItemId;
    Array*      itemPrototypes;
    HashTable*  scriptPathToDbEntry;
    Array*      changes;
    lua_State*  L;
};

STRUCT_DEFINE(ItemDbEntry)
{
    uint32_t    itemId;
    int         isUpdate;
    uint64_t    timestamp;
    char*       addPath;
};

void    item_gen_init(ItemGen* gen);
void    item_gen_deinit(ItemGen* gen);

void    item_gen_scan_and_generate(ItemGen* gen);

EQP_API void    item_gen_add(ItemGen* gen, ItemPrototype* proto, const char* scriptPath, uint32_t len, uint64_t timestamp);

#endif//EQP_ITEM_GEN_H
