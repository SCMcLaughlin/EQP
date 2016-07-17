
#include "item_shm_loader.h"
#include "eqp_master.h"

void item_shm_loader_init(ItemShmLoader* loader)
{
    loader->isReplaceInProgress = false;
    
    shm_creator_init(&loader->current.creator);
    shm_viewer_init(&loader->current.viewer);
    
    shm_creator_init(&loader->replacement.creator);
    shm_viewer_init(&loader->replacement.viewer);
}

void item_shm_loader_deinit(ItemShmLoader* loader)
{
    // Don't use share_mem_destroy, because that would unlink the file on disc
    shm_viewer_close(&loader->current.viewer);
    shm_viewer_close(&loader->replacement.viewer);
}

static int item_shm_loader_try_open(Master* M, ItemShmLoaderSet* set, const char* path)
{
    FILE* fp = fopen(path, "rb");
    uint32_t length;
    
    if (!fp)
        goto ret_false;
    
    length = file_calc_size(fp);
    fclose(fp);
    
    if (length == 0)
        goto ret_false;
    
    share_mem_create_open(B(M), &set->creator, &set->viewer, path, length);
    
ret_false:
    return false;
}

void item_shm_loader_open_main(Master* M, ItemShmLoader* loader)
{
    if (!item_shm_loader_try_open(M, &loader->current, EQP_ITEM_SHM_PATH1) &&
        !item_shm_loader_try_open(M, &loader->current, EQP_ITEM_SHM_PATH2))
        log_format(B(M), LogWarning, "Could not open ItemPrototype shared memory file");
}

void item_shm_loader_send_details(Master* M, ItemShmLoader* loader, ChildProcess* proc)
{
    byte buf[1024];
    Server_ItemSharedMemoryOpen* data = (Server_ItemSharedMemoryOpen*)buf;
    int len;
    
    if (!shm_viewer_memory(&loader->current.viewer))
        return;
    
    data->length = shm_viewer_memory_length(&loader->current.viewer);
    len = snprintf(data->path, sizeof(buf) - sizeof_field(Server_ItemSharedMemoryOpen, length), "%s", share_mem_path(&loader->current.creator));
    
    len += sizeof_field(Server_ItemSharedMemoryOpen, length) + 1;
    
    proc_ipc_send(B(M), proc, ServerOp_ItemSharedMemoryOpen, EQP_SOURCE_ID_MASTER, len, data);
}
