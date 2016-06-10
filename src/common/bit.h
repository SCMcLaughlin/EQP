
#ifndef EQP_BIT_H
#define EQP_BIT_H

#include "define.h"

#define bit_mask(n) ((1 << (n)) - 1)
#define bit_mask64(n) ((1ULL << (n)) - 1ULL)
#define bit_is_pow2(n) ((n) && (((n) & ((n) - 1)) == 0))
#define bit_rotate(x, n) (((x)<<(n)) | ((x)>>(-(int)(n)&(8*sizeof(x)-1))))
#define bit_get(val, n) (val & (1 << n))
#define bit_get64(val, n) (val & (1ULL << (n)))

int         bit_next_pow2_i32(int n);
uint32_t    bit_next_pow2_u32(uint32_t n);
int64_t     bit_next_pow2_i64(int64_t n);
uint64_t    bit_next_pow2_u64(uint64_t n);

#define bit_next_pow2_selector(n) _Generic((n), \
    default:    bit_next_pow2_i32,              \
    uint32_t:   bit_next_pow2_u32,              \
    int64_t:    bit_next_pow2_i64,              \
    uint64_t:   bit_next_pow2_u64               \
    )(n)

#define bit_pow2_greater_than(n) ((bit_is_pow2(n)) ? ((n) << 1) : (bit_next_pow2_selector((n))))
#define bit_pow2_greater_or_equal(n) ((bit_is_pow2(n)) ? (n) : (bit_next_pow2_selector((n))))

#endif//EQP_BIT_H