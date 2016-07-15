
#include "item_share_mem.h"

static uint32_t item_share_mem_hash(uint32_t lo, uint32_t hi)
{
    lo ^= hi;
    hi = bit_rotate(hi, 14);
    lo -= hi;
    hi = bit_rotate(hi, 5);
    hi ^= lo;
    hi -= bit_rotate(lo, 13);
    return hi;
}

uint32_t item_share_mem_calc_hash(uint32_t itemId)
{
    return item_share_mem_hash(0xffccff44, itemId);
}
