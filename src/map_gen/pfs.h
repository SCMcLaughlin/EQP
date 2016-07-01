
#ifndef EQP_PFS_H
#define EQP_PFS_H

#include "define.h"
#include "eqp_array.h"
#include "eqp_hash_table.h"
#include "eqp_string.h"
#include "eqp_alloc.h"
#include <zlib.h>

STRUCT_DECLARE(Basic);

STRUCT_DEFINE(Pfs)
{
    String*     path;
    Array*      fileEntries;
    HashTable*  fileEntriesByName;
    Basic*      basic;
    byte*       data;
    uint32_t    length;
};

void    pfs_open(Basic* basic, Pfs* pfs, const char* path);
void    pfs_close(Pfs* pfs);

int     pfs_has_file_by_name(Pfs* pfs, const char* name);
int     pfs_is_s3d_zone(Pfs* pfs);
byte*   pfs_get_file_by_name(Pfs* pfs, const char* name, uint32_t* fileLength);

#endif//EQP_PFS_H
