
#ifndef EQP_ALIGNED_H
#define EQP_ALIGNED_H

#include "define.h"
#include "random.h"
#include "eqp_string.h"

STRUCT_DECLARE(Basic);

STRUCT_DEFINE(Aligned)
{
    uint32_t    cursor;
    uint32_t    length;
    byte*       buffer;
    Basic*      basic;
};

void        aligned_init(R(Basic*) basic, R(Aligned*) a, R(void*) ptr, uint32_t len);
void        aligned_init_cursor(R(Basic*) basic, R(Aligned*) a, R(void*) ptr, uint32_t len, uint32_t cursor);
void        aligned_init_copy(R(Aligned*) dst, R(Aligned*) src);
uint32_t    aligned_advance(R(Aligned*) a, uint32_t len);
uint32_t    aligned_reverse(R(Aligned*) a, uint32_t len);
void        aligned_check(R(Aligned*) a, uint32_t len);
// Returns the number of bytes advanced over, not counting the null terminator, or -1 if there is no null terminator before the end
int         aligned_advance_null_terminator(R(Aligned*) a);
#define     aligned_size(a) ((a)->length)
#define     aligned_remaining(a) ((a)->length - (a)->cursor)
#define     aligned_position(a) ((a)->cursor)
#define     aligned_all(a) ((a)->buffer)
#define     aligned_current(a) ((a)->buffer + (a)->cursor)
#define     aligned_current_type(a, type) (type*)aligned_current(a)
#define     aligned_at_position(a, pos) ((a)->buffer + (pos))

#define     aligned_set_basic(a, b) ((a)->basic = (b))

#define     aligned_reduce_size(a, by) ((a)->length -= (by))
#define     aligned_reset(a) ((a)->cursor = 0)
#define     aligned_reset_to(a, offset) ((a)->cursor = (offset))
void        aligned_reinit(R(Aligned*) a, R(void*) ptr, uint32_t len);
void        aligned_reinit_cursor(R(Aligned*) a, R(void*) ptr, uint32_t len, uint32_t cursor);

// Read
uint8_t     aligned_read_uint8(R(Aligned*) a);
#define     aligned_read_int8(a) ((int8_t)aligned_read_uint8((a)))
#define     aligned_read_byte(a) (aligned_read_uint8((a)))
uint16_t    aligned_read_uint16(R(Aligned*) a);
#define     aligned_read_int16(a) ((int16_t)aligned_read_uint16((a)))
uint32_t    aligned_read_uint32(R(Aligned*) a);
#define     aligned_read_int32(a) ((int32_t)aligned_read_uint32((a)))
#define     aligned_read_int(a) aligned_read_int32(a)
uint64_t    aligned_read_uint64(R(Aligned*) a);
#define     aligned_read_int64(a) ((int64_t)aligned_read_uint64((a)))
void        aligned_read_buffer(R(Aligned*) a, R(void*) dst, uint32_t len);

uint8_t     aligned_peek_uint8(R(Aligned*) a);
#define     aligned_peek_int8(a) ((int8_t)aligned_peek_uint8((a)))
#define     aligned_peek_byte(a) (aligned_peek_uint8((a)))

// Write
#define     aligned_write_zero_all(a) (memset((a)->buffer, 0, (a)->length))
void        aligned_write_uint8(R(Aligned*) a, uint8_t v);
#define     aligned_write_byte(a, v) aligned_write_uint8((a), (v))
#define     aligned_write_int8(a, v) (aligned_write_uint8((a), (uint8_t)(v)))
#define     aligned_write_bool(a, v) (aligned_write_uint8((a), (v) ? 1 : 0))
void        aligned_write_uint16(R(Aligned*) a, uint16_t v);
#define     aligned_write_int16(a, v) (aligned_write_uint16((a), (uint16_t)(v)))
void        aligned_write_uint32(R(Aligned*) a, uint32_t v);
#define     aligned_write_int32(a, v) (aligned_write_uint32((a), (uint32_t)(v)))
#define     aligned_write_int(a, v) aligned_write_int32(a, v)
void        aligned_write_uint64(R(Aligned*) a, uint64_t v);
#define     aligned_write_int64(a, v) (aligned_write_uint64((a), (uint64_t)(v)))
void        aligned_write_buffer(R(Aligned*) a, R(const void*) data, uint32_t len);
#define     aligned_write_string_null_terminated(a, str) aligned_write_buffer((a), string_data(str), (string_length(str) + 1))
#define     aligned_write_literal_null_terminated(a, str) aligned_write_buffer((a), str, sizeof(str))
void        aligned_write_random(R(Aligned*) a, int bytes);
void        aligned_write_zeroes(R(Aligned*) a, uint32_t count);
// Always advances n characters, regardless of how many are actually written
void        aligned_write_snprintf_full_advance(R(Aligned*) a, uint32_t n, R(const char*) fmt, ...);

void        aligned_write_reverse_uint8(R(Aligned*) a, uint8_t v);
void        aligned_write_reverse_uint16(R(Aligned*) a, uint16_t v);

#endif//EQP_ALIGNED_H
