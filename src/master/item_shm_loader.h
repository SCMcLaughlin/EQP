
#ifndef EQP_ITEM_SHM_LOADER_H
#define EQP_ITEM_SHM_LOADER_H

#include "define.h"
#include "share_mem.h"
#include "child_process.h"
#include "file.h"
#include "source_id.h"
#include "server_op.h"
#include "server_structs.h"

STRUCT_DECLARE(Master);

STRUCT_DEFINE(ItemShmLoaderSet)
{
    ShmCreator  creator;
    ShmViewer   viewer;
};

STRUCT_DEFINE(ItemShmLoader)
{
    int                 isReplaceInProgress;
    ItemShmLoaderSet    current;
    ItemShmLoaderSet    replacement;
};

void    item_shm_loader_init(ItemShmLoader* loader);
void    item_shm_loader_deinit(ItemShmLoader* loader);

void    item_shm_loader_open_main(Master* M, ItemShmLoader* loader);
void    item_shm_loader_send_details(Master* M, ItemShmLoader* loader, ChildProcess* proc);

#endif//EQP_ITEM_SHM_LOADER_H
