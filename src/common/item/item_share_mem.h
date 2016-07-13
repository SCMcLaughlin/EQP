
#ifndef EQP_ITEM_SHARE_MEM_H
#define EQP_ITEM_SHARE_MEM_H

#include "define.h"
#include "share_mem.h"
#include "item_prototype.h"

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
    uint32_t        padding;
    ItemShm_Entry   entries[0];
};

typedef ShmViewer ItemShareMem;

void            item_share_mem_open(ItemShareMem* shm, const char* path, uint32_t length);
#define         item_share_mem_close(itemshm) shm_viewer_close(itemshm)
ItemPrototype*  item_share_mem_get_prototype(ItemShareMem* shm, uint32_t itemId);

#endif//EQP_ITEM_SHARE_MEM_H
