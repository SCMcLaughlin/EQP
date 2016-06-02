
#include "share_mem.h"

void shm_viewer_init(R(ShmViewer*) viewer)
{
    viewer->memory = NULL;
    viewer->length = 0;
    
#ifdef EQP_WINDOWS
    viewer->handle = INVALID_HANDLE_VALUE;
#endif
}

void shm_viewer_open(R(Basic*) basic, R(ShmViewer*) viewer, R(const char*) path, uint32_t length)
{
#ifdef EQP_WINDOWS
    viewer->handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, path);
    
    if (viewer->handle == NULL)
    {
        viewer->handle = INVALID_HANDLE_VALUE;
        exception_throw(basic, ErorrShareMemInit);
    }
    
    viewer->memory = MapViewOfFile(viewer->handle, FILE_MAP_ALL_ACCESS, 0, 0, length);
#else
    int fd = open(path, O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    
    if (fd == -1)
        exception_throw_format(basic, ErrorShareMemInit, "[shm_viewer_open] Could not create or open '%s', errno: %i", path, errno);
    
    viewer->memory = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    close(fd);
#endif
    
    if (viewer->memory == NULL)
        exception_throw_format(basic, ErrorShareMemInit, "[shm_viewer_open] memory mapping failed for '%s', errno: %i", path, errno);
    
    viewer->length = length;
}

void shm_viewer_close(R(ShmViewer*) viewer)
{
    if (viewer->memory)
    {
#ifdef EQP_WINDOWS
        UnmapViewOfFile(viewer->memory);
#else
        munmap(viewer->memory, viewer->length);
#endif
        viewer->memory = NULL;
    }
    
#ifdef EQP_WINDOWS
    if (viewer->handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(viewer->handle);
        viewer->handle = INVALID_HANDLE_VALUE;
    }
#endif
}

static void shm_pick_name(R(ShmCreator*) creator, R(const char*) pathBase)
{
    for (;;)
    {
        FILE* fp;
        
        snprintf(creator->name, sizeof(creator->name), "%s%lu", pathBase, random_uint64());
        
        fp = fopen(creator->name, "r");
        
        if (!fp)
            return;
        
        fclose(fp);
    }
}

void share_mem_create(R(Basic*) basic, R(ShmCreator*) creator, R(ShmViewer*) viewer, R(const char*) pathBase, uint32_t length)
{
#ifdef EQP_LINUX
    int fd;
#endif
    
    shm_pick_name(creator, pathBase);
    
#ifdef EQP_WINDOW
    viewer->handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, length, creator->name);
    
    if (viewer->handle == NULL)
    {
        viewer->handle = INVALID_HANDLE_VALUE;
        exception_throw(basic, ErorrShareMemInit);
    }
    
    viewer->memory = MapViewOfFile(viewer->handle, FILE_MAP_ALL_ACCESS, 0, 0, length);
#else
    fd = open(creator->name, O_CREAT | O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    
    if (fd == -1)
        exception_throw_format(basic, ErrorShareMemInit, "[share_mem_create] Could not create or open '%s', errno: %i", creator->name, errno);
    
    if (ftruncate(fd, length))
    {
        close(fd);
        exception_throw_format(basic, ErrorShareMemInit, "[share_mem_create] ftruncate() failed for '%s', errno: %i", creator->name, errno);
    }
    
    viewer->memory = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    close(fd);
#endif
    
    if (viewer->memory == NULL)
        exception_throw_format(basic, ErrorShareMemInit, "[share_mem_create] memory mapping failed for '%s', errno: %i", creator->name, errno);
    
    viewer->length = length;
}

void share_mem_destroy(R(ShmCreator*) creator, R(ShmViewer*) viewer)
{
#ifdef EQP_LINUX
    if (viewer->memory)
    {
        shm_viewer_close(viewer);
        unlink(creator->name);
    }
#endif
}
