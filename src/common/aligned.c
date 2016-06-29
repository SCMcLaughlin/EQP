
#include "aligned.h"
#include "eqp_basic.h"
#include "exception.h"

#define ERR_ADVANCE "[aligned_advance] Attempt to advance beyond end of buffer"
#define ERR_REVERSE "[aligned_reverse] Attempt to advance below beginning of buffer"
#define ERR_CHECK "[aligned_check] Attempt to check beyond end of buffer"

void aligned_init(Basic* basic, Aligned* a, void* ptr, uint32_t len)
{
    a->cursor   = 0;
    a->length   = len;
    a->buffer   = (byte*)ptr;
    a->basic    = basic;
}

void aligned_init_cursor(Basic* basic, Aligned* a, void* ptr, uint32_t len, uint32_t cursor)
{
    a->cursor   = cursor;
    a->length   = len;
    a->buffer   = (byte*)ptr;
    a->basic    = basic;
}

void aligned_init_copy(Aligned* dst, Aligned* src)
{
    memcpy(dst, src, sizeof(Aligned));
}

uint32_t aligned_advance(Aligned* a, uint32_t len)
{
    uint32_t c = a->cursor;
    a->cursor += len;
    
    if (a->cursor > a->length)
        exception_throw_literal(a->basic, ErrorOutOfBounds, ERR_ADVANCE);
    
    return c;
}

uint32_t aligned_reverse(Aligned* a, uint32_t len)
{
    uint32_t c = a->cursor;
    int cursor = ((int)a->cursor) - ((int)len);
    
    if (cursor < 0)
        exception_throw_literal(a->basic, ErrorOutOfBounds, ERR_REVERSE);
    
    a->cursor = (uint32_t)cursor;
    
    return c;
}

void aligned_check(Aligned* a, uint32_t len)
{
    uint32_t c = a->cursor + len;
    
    if (c > a->length)
        exception_throw_literal(a->basic, ErrorOutOfBounds, ERR_CHECK);
}

int aligned_advance_null_terminator(Aligned* a)
{
    int len         = 0;
    uint32_t length = a->length;
    byte* buffer    = a->buffer;
    
    while (a->cursor < length)
    {
        byte c = buffer[a->cursor++];
        
        if (c == 0)
            return len;
        
        len++;
    }
    
    return -1;
}

void aligned_reinit(Aligned* a, void* ptr, uint32_t len)
{
    a->cursor   = 0;
    a->length   = len;
    a->buffer   = (byte*)ptr;
}

void aligned_reinit_cursor(Aligned* a, void* ptr, uint32_t len, uint32_t cursor)
{
    a->cursor   = cursor;
    a->length   = len;
    a->buffer   = (byte*)ptr;
}

// Read
uint8_t aligned_read_uint8(Aligned* a)
{
    uint32_t c = aligned_advance(a, sizeof(uint8_t));
    return a->buffer[c];
}

uint16_t aligned_read_uint16(Aligned* a)
{
    uint32_t c      = aligned_advance(a, sizeof(uint16_t));
    byte* buffer    = a->buffer;
    uint16_t ret;
    
    ret  = buffer[c + 0] << 0;
    ret |= buffer[c + 1] << 8;
    
    return ret;
}

uint32_t aligned_read_uint32(Aligned* a)
{
    uint32_t c      = aligned_advance(a, sizeof(uint32_t));
    byte* buffer    = a->buffer;
    uint32_t ret;
    
    ret  = buffer[c + 0] <<  0;
    ret |= buffer[c + 1] <<  8;
    ret |= buffer[c + 2] << 16;
    ret |= buffer[c + 3] << 24;
    
    return ret;
}

uint64_t aligned_read_uint64(Aligned* a)
{
    uint32_t c      = aligned_advance(a, sizeof(uint64_t));
    byte* buffer    = a->buffer;
    uint64_t ret;
    
    ret  = ((uint64_t)buffer[c + 0]) <<  0;
    ret |= ((uint64_t)buffer[c + 1]) <<  8;
    ret |= ((uint64_t)buffer[c + 2]) << 16;
    ret |= ((uint64_t)buffer[c + 3]) << 24;
    ret |= ((uint64_t)buffer[c + 4]) << 32;
    ret |= ((uint64_t)buffer[c + 5]) << 40;
    ret |= ((uint64_t)buffer[c + 6]) << 48;
    ret |= ((uint64_t)buffer[c + 7]) << 56;
    
    return ret;
}

void aligned_read_buffer(Aligned* a, void* ptr, uint32_t len)
{
    byte* dst   = (byte*)ptr;
    byte* src   = a->buffer;
    uint32_t c  = aligned_advance(a, len);
    uint32_t i;
    
    for (i = 0; i < len; i++)
    {
        dst[i] = src[c + i];
    }
}

uint8_t aligned_peek_uint8(Aligned* a)
{
    aligned_check(a, sizeof(uint8_t));
    return a->buffer[a->cursor];
}

// Write
void aligned_write_memset(Aligned* a, int val, uint32_t len)
{
    uint32_t c = aligned_advance(a, len);
    memset(a->buffer + c, val, len);
}

void aligned_write_uint8(Aligned* a, uint8_t v)
{
    uint32_t c = aligned_advance(a, sizeof(uint8_t));
    a->buffer[c] = v;
}

void aligned_write_uint16(Aligned* a, uint16_t v)
{
    uint32_t c      = aligned_advance(a, sizeof(uint16_t));
    byte* buffer    = a->buffer;
    
    buffer[c + 0] = (uint8_t)((v & 0x00ff) >> 0);
    buffer[c + 1] = (uint8_t)((v & 0xff00) >> 8);
}

void aligned_write_uint32(Aligned* a, uint32_t v)
{
    uint32_t c      = aligned_advance(a, sizeof(uint32_t));
    byte* buffer    = a->buffer;
    
    buffer[c + 0] = (uint8_t)((v & 0x000000ff) >>  0);
    buffer[c + 1] = (uint8_t)((v & 0x0000ff00) >>  8);
    buffer[c + 2] = (uint8_t)((v & 0x00ff0000) >> 16);
    buffer[c + 3] = (uint8_t)((v & 0xff000000) >> 24);
}

void aligned_write_uint64(Aligned* a, uint64_t v)
{
    uint32_t c      = aligned_advance(a, sizeof(uint64_t));
    byte* buffer    = a->buffer;
    
    buffer[c + 0] = (uint8_t)((v & 0x00000000000000ff) >>  0);
    buffer[c + 1] = (uint8_t)((v & 0x000000000000ff00) >>  8);
    buffer[c + 2] = (uint8_t)((v & 0x0000000000ff0000) >> 16);
    buffer[c + 3] = (uint8_t)((v & 0x00000000ff000000) >> 24);
    buffer[c + 4] = (uint8_t)((v & 0x000000ff00000000) >> 32);
    buffer[c + 5] = (uint8_t)((v & 0x0000ff0000000000) >> 40);
    buffer[c + 6] = (uint8_t)((v & 0x00ff000000000000) >> 48);
    buffer[c + 7] = (uint8_t)((v & 0xff00000000000000) >> 56);
}

void aligned_write_float(Aligned* a, float v)
{
    uint32_t u = *(uint32_t*)&v;
    aligned_write_uint32(a, u);
}

void aligned_write_buffer(Aligned* a, const void* data, uint32_t len)
{
    uint32_t c      = aligned_advance(a, len);
    byte* dst       = a->buffer;
    const byte* src = (const byte*)data;
    uint32_t i;
    
    for (i = 0; i < len; i++)
    {
        dst[c + i] = src[i];
    }
}

void aligned_write_random(Aligned* a, int bytes)
{
    uint32_t c = aligned_advance(a, (uint32_t)bytes);
    random_bytes(a->buffer + c, bytes);
}

void aligned_write_zeroes(Aligned* a, uint32_t count)
{
    uint32_t c = aligned_advance(a, count);
    memset(a->buffer + c, 0, count);
}

void aligned_write_snprintf_full_advance(Aligned* a, uint32_t n, const char* fmt, ...)
{
    uint32_t c = aligned_advance(a, n);
    va_list args;
    
    va_start(args, fmt);
    vsnprintf((char*)(a->buffer + c), n, fmt, args);
    va_end(args);
}

void aligned_write_reverse_uint8(Aligned* a, uint8_t v)
{
    uint32_t c = aligned_reverse(a, sizeof(uint8_t));
    a->buffer[c - 1] = v;
}

void aligned_write_reverse_uint16(Aligned* a, uint16_t v)
{
    uint32_t c      = aligned_reverse(a, sizeof(uint16_t));
    byte* buffer    = a->buffer;
    
    buffer[c - 2] = (uint8_t)((v & 0x00ff) >> 0);
    buffer[c - 1] = (uint8_t)((v & 0xff00) >> 8);
}

#undef ERR_ADVANCE
#undef ERR_REVERSE
#undef ERR_CHECK
