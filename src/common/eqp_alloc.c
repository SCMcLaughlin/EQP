
#include "eqp_alloc.h"
#include "eqp_basic.h"

void* eqp_realloc(R(Basic*) basic, R(void*) ptr, size_t len)
{
    ptr = realloc(ptr, len);
    
    if (ptr == NULL)
        exception_throw(basic, ErrorOutOfMemory);
    
    return ptr;
}
