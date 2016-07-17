
#ifndef EQP_SHARE_MEM_H
#define EQP_SHARE_MEM_H

#include "define.h"
#include "define_share_mem.h"
#include "eqp_basic.h"
#include "exception.h"
#include "random.h"

STRUCT_DEFINE(ShmViewer)
{
    void*       memory;
    uint32_t    length;
    
#ifdef EQP_WINDOWS
    HANDLE      handle;
#endif
};

STRUCT_DEFINE(ShmCreator)
{
    char name[64];
};

void    shm_viewer_init(ShmViewer* viewer);
void    shm_viewer_open(Basic* basic, ShmViewer* viewer, const char* path, uint32_t length);
void    shm_viewer_close(ShmViewer* viewer);
#define shm_viewer_memory(viewer) ((viewer)->memory)
#define shm_viewer_memory_type(viewer, type) (type*)((viewer)->memory)
#define shm_viewer_memory_length(viewer) ((viewer)->length)

#define shm_creator_init(creator) memset(creator, 0, sizeof(ShmCreator))

void    share_mem_create(Basic* basic, ShmCreator* creator, ShmViewer* viewer, const char* pathBase, uint32_t length);
void    share_mem_create_open(Basic* basic, ShmCreator* creator, ShmViewer* viewer, const char* path, uint32_t length);
void    share_mem_destroy(ShmCreator* creator, ShmViewer* viewer);
#define share_mem_path(shm) ((shm)->name)

#endif//EQP_SHARE_MEM_H
