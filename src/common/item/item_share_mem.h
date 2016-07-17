
#ifndef EQP_ITEM_SHARE_MEM_H
#define EQP_ITEM_SHARE_MEM_H

#include "define.h"
#include "bit.h"
#include "share_mem.h"
#include "item_prototype.h"
#include "lua_sys.h"

STRUCT_DECLARE(Basic);

STRUCT_DEFINE(ItemShm_Entry)
{
    uint32_t    key;
    uint32_t    hash;
    uint32_t    next;
    uint32_t    offset;
};

STRUCT_DEFINE(ItemShm)
{
    uint64_t        timestamp;
    uint32_t        capacityMinusOne;
    uint32_t        itemCount;
    ItemShm_Entry   entries[0];
};

typedef ShmViewer ItemShareMem;

#define         item_share_mem_init(shm) shm_viewer_init((shm))
void            item_share_mem_open(Basic* basic, lua_State* L, ItemShareMem* shm, const char* path, uint32_t length);
#define         item_share_mem_close(itemshm) shm_viewer_close(itemshm)
ItemPrototype*  item_share_mem_get_prototype(ItemShareMem* shm, uint32_t itemId);
ItemPrototype*  item_share_mem_get_array(ItemShareMem* shm);
uint32_t        item_share_mem_get_array_count(ItemShareMem* shm);

uint32_t        item_share_mem_calc_hash(uint32_t itemId);

#endif//EQP_ITEM_SHARE_MEM_H
