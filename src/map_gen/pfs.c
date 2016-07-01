
#include "pfs.h"
#include "eqp_basic.h"

#define SIGNATURE   0x20534650 /* "PFS " */

STRUCT_DEFINE(Pfs_Header)
{
    uint32_t    offset;
    uint32_t    signature;
    uint32_t    unknown;
};

STRUCT_DEFINE(Pfs_BlockHeader)
{
    uint32_t    deflatedLength;
    uint32_t    inflatedLength;
};

STRUCT_DEFINE(Pfs_DirEntry)
{
    uint32_t    crc;
    uint32_t    offset;
    uint32_t    inflatedLength;
};

STRUCT_DEFINE(Pfs_Entry)
{
    uint32_t    crc;
    uint32_t    offset;
    uint32_t    deflatedLength;
    uint32_t    inflatedLength;
};

static byte* pfs_open_file(Basic* basic, const char* path, uint32_t* length)
{
    FILE* fp = fopen(path, "rb");
    uint32_t len;
    byte* data;
    
    if (!fp)
        exception_throw(basic, ErrorDoesNotExist);
    
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    
    if (len == 0)
    {
        fclose(fp);
        exception_throw(basic, ErrorNoData);
    }
    
    rewind(fp);
    data = eqp_alloc_type_bytes(basic, len, byte);
    fread(data, 1, len, fp);
    fclose(fp);
    
    *length = len;
    return data;
}

static void pfs_init(Basic* basic, Pfs* pfs, const char* path, byte* data, uint32_t length)
{
    pfs->path               = string_create_from_cstr(basic, path, strlen(path));
    pfs->fileEntries        = array_create_type(basic, Pfs_Entry);
    pfs->fileEntriesByName  = hash_table_create_type(basic, uint32_t);
    pfs->basic              = basic;
    pfs->data               = data;
    pfs->length             = length;
}

static int pfs_sort(const void* a, const void* b)
{
    Pfs_Entry* entA = (Pfs_Entry*)a;
    Pfs_Entry* entB = (Pfs_Entry*)b;
    
    return (entA->offset < entB->offset) ? -1 : 1;
}

static byte* pfs_decompress_entry(Pfs* pfs, uint32_t index, uint32_t* length)
{
    Pfs_Entry* ent = array_get_type(pfs->fileEntries, index, Pfs_Entry);
    byte* src;
    byte* buffer;
    uint32_t ilen;
    uint32_t read;
    uint32_t pos;
    Basic* basic = pfs->basic;
    
    if (!ent || ent->offset > pfs->length || (ent->offset + ent->deflatedLength) > pfs->length)
        exception_throw(basic, ErrorOutOfBounds);
    
    src     = pfs->data + ent->offset;
    ilen    = ent->inflatedLength;
    read    = 0;
    pos     = 0;
    buffer  = eqp_alloc_type_bytes(basic, ilen, byte);
    
    while (read < ilen)
    {
        unsigned long len = ilen - read;
        Pfs_BlockHeader* bh = (Pfs_BlockHeader*)(src + pos);
        pos += sizeof(Pfs_BlockHeader);
        
        if (uncompress(buffer + read, &len, src + pos, bh->deflatedLength) != Z_OK)
            goto failure;
        
        read += bh->inflatedLength;
        pos  += bh->deflatedLength;
    }
    
    if (length)
        *length = ilen;
    
    return buffer;
    
failure:
    free(buffer);
    exception_throw(basic, ErrorCompression);
    return NULL; // Unreachable
}

void pfs_open(Basic* basic, Pfs* pfs, const char* path)
{
    uint32_t len;
    uint32_t p;
    uint32_t n;
    uint32_t i;
    byte* data;
    Pfs_Header* h;
    
    data    = pfs_open_file(basic, path, &len);
    p       = sizeof(Pfs_Header);
    
    pfs_init(basic, pfs, path, data, len);
    
    if (p > len)
        exception_throw(basic, ErrorOutOfBounds);
    
    h = (Pfs_Header*)data;
    
    if (h->signature != SIGNATURE)
        exception_throw(basic, ErrorMismatch);
    
    p = h->offset;
    n = *(uint32_t*)(data + p);
    
    p += sizeof(uint32_t);

    for (i = 0; i < n; i++)
    {
        Pfs_DirEntry* src = (Pfs_DirEntry*)(data + p);
        Pfs_Entry ent;
        uint32_t q;
        uint32_t ilen;
        uint32_t totalLen;
        
        p += sizeof(Pfs_DirEntry);
        
        if (p > len)
            exception_throw(basic, ErrorOutOfBounds);
        
        ent.crc             = src->crc;
        ent.offset          = src->offset;
        ent.inflatedLength  = src->inflatedLength;
        
        q = ent.offset;
        
        if (q > len)
            exception_throw(basic, ErrorOutOfBounds);
        
        ilen        = 0;
        totalLen    = ent.inflatedLength;
        
        while (ilen < totalLen)
        {
            Pfs_BlockHeader* bh = (Pfs_BlockHeader*)(data + q);
            q += sizeof(Pfs_BlockHeader);
            
            if (q > len)
                exception_throw(basic, ErrorOutOfBounds);
            
            q += bh->deflatedLength;
            
            if (q > len)
                exception_throw(basic, ErrorOutOfBounds);
            
            ilen += bh->inflatedLength;
        }
        
        ent.deflatedLength = q - ent.offset;
        
        array_push_back(basic, &pfs->fileEntries, &ent);
    }
    
    // Must have file names entry and some kind of content, or it may as well not exist
    if (array_count(pfs->fileEntries) < 2)
        exception_throw(basic, ErrorNoData);
    
    qsort(array_data(pfs->fileEntries), array_count(pfs->fileEntries), array_element_size(pfs->fileEntries), pfs_sort);
    
    // Retrieve file names entry
    data = pfs_decompress_entry(pfs, array_count(pfs->fileEntries) - 1, &len);
    array_pop_back(pfs->fileEntries);
    
    n = *(uint32_t*)data;
    p = sizeof(uint32_t);
    
    if (p > len)
        exception_throw(basic, ErrorOutOfBounds);
    
    for (i = 0; i < n; i++)
    {
        uint32_t namelen;
        const char* name;
        
        namelen = *(uint32_t*)(data + p);
        p += sizeof(uint32_t);
        
        if (p > len)
            exception_throw(basic, ErrorOutOfBounds);
        
        name = (const char*)(data + p);
        p += namelen; // Includes null terminator
        
        if (p > len)
            exception_throw(basic, ErrorOutOfBounds);
        
        hash_table_set_by_cstr(basic, &pfs->fileEntriesByName, name, namelen - 1, &i);
    }

    free(data);
}

void pfs_close(Pfs* pfs)
{
    if (pfs->path)
    {
        string_destroy(pfs->path);
        pfs->path = NULL;
    }
    
    if (pfs->fileEntries)
    {
        array_destroy(pfs->fileEntries);
        pfs->fileEntries = NULL;
    }
    
    if (pfs->fileEntriesByName)
    {
        hash_table_destroy(pfs->fileEntriesByName);
        pfs->fileEntriesByName = NULL;
    }
    
    if (pfs->data)
    {
        free(pfs->data);
        pfs->data = NULL;
    }
}

int pfs_has_file_by_name(Pfs* pfs, const char* name)
{
    uint32_t* index = hash_table_get_type_by_cstr(pfs->fileEntriesByName, name, strlen(name), uint32_t);
    return (index != NULL);
}

int pfs_is_s3d_zone(Pfs* pfs)
{
    return pfs_has_file_by_name(pfs, "objects.wld");
}

byte* pfs_get_file_by_name(Pfs* pfs, const char* name, uint32_t* fileLength)
{
    uint32_t* index = hash_table_get_type_by_cstr(pfs->fileEntriesByName, name, strlen(name), uint32_t);
    
    if (!index)
        exception_throw(pfs->basic, ErrorMismatch);
    
    return pfs_decompress_entry(pfs, *index, fileLength);
}

#undef SIGNATURE
