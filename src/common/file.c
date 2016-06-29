
#include "file.h"

uint64_t file_calc_size(FILE* fp)
{
    uint64_t size;
    
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    
    return size;
}
