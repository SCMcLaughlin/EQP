
#include "file.h"

uint64_t file_calc_size(FILE* fp)
{
    uint64_t size;
    
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    return size;
}

uint64_t file_calc_size_remaining(FILE* fp)
{
    uint64_t cur;
    uint64_t size;
    
    cur = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size = ftell(fp) - cur;
    fseek(fp, cur, SEEK_SET);
    
    return size;
}

byte* file_read_remaining(Basic* basic, FILE* fp, uint32_t* len)
{
    uint64_t cur    = ftell(fp);
    uint64_t length = file_calc_size_remaining(fp);
    byte* buffer;
    
    if (length == 0)
    {
        if (len)
            *len = 0;
        return NULL;
    }
    
    buffer = eqp_alloc_type_bytes(basic, length, byte);
    
    fread(buffer, 1, length, fp);
    fseek(fp, cur, SEEK_SET);
    
    if (len)
        *len = length;
    
    return buffer;
}

byte* file_read_remaining_decompress_no_throw(Basic* basic, FILE* fp, uint32_t* len, uint32_t inflatedLength)
{
    uint32_t length;
    byte* compressed;
    byte* decompressed;
    unsigned long dlen;
    int rc;
    
    compressed = file_read_remaining(basic, fp, &length);
    
    if (!compressed)
        goto ret_null;
    
    dlen            = inflatedLength;
    decompressed    = eqp_alloc_type_bytes(basic, dlen, byte);
    rc              = uncompress(decompressed, &dlen, compressed, length);
    
    if (rc != Z_OK)
    {
        log_format(basic, LogWarning, "[file_read_remaining_decompress_no_throw] decompression failed, error code %i", rc);
        free(compressed);
        free(decompressed);
        goto ret_null;
    }
    
    free(compressed);
    
    if (len)
        *len = dlen;
    
    return decompressed;
    
ret_null:
    if (len)
        *len = 0;
    
    return NULL;
}
