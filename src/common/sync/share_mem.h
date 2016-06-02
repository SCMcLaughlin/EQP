
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

void    shm_viewer_init(R(ShmViewer*) viewer);
void    shm_viewer_open(R(Basic*) basic, R(ShmViewer*) viewer, R(const char*) path, uint32_t length);
void    shm_viewer_close(R(ShmViewer*) viewer);
#define shm_viewer_memory_type(viewer, type) (type*)((viewer)->memory)

void    share_mem_create(R(Basic*) basic, R(ShmCreator*) creator, R(ShmViewer*) viewer, R(const char*) pathBase, uint32_t length);
void    share_mem_destroy(R(ShmCreator*) creator, R(ShmViewer*) viewer);
#define share_mem_path(shm) ((shm)->name)

#endif//EQP_SHARE_MEM_H
