
#include "bit.h"

int bit_next_pow2_i32(int n)
{
    return (int)bit_next_pow2_u32((uint32_t)n);
}

uint32_t bit_next_pow2_u32(uint32_t n)
{
    n--;
    
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    
    n++;
    
    return n;
}

int64_t bit_next_pow2_i64(int64_t n)
{
    return (int64_t)bit_next_pow2_u64((uint64_t)n);
}

uint64_t bit_next_pow2_u64(uint64_t n)
{
    n--;
    
    n |= n >> 1ULL;
    n |= n >> 2ULL;
    n |= n >> 4ULL;
    n |= n >> 8ULL;
    n |= n >> 16ULL;
    n |= n >> 32ULL;
    
    n++;
    
    return n;
}
