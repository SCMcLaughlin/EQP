
#include "eqp_string.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"
#include "file.h"

#define MIN_CAPACITY 8

STRUCT_DEFINE(String)
{
    uint32_t    length;
    uint32_t    capacity;
    int         refCount;
    char        data[0];
};

String* string_create(Basic* basic)
{
    String* str     = eqp_alloc_type_bytes(basic, sizeof(String) + MIN_CAPACITY, String);
    
    str->length     = 0;
    str->capacity   = MIN_CAPACITY;
    str->refCount   = 1;
    
    return str;
}

String* string_create_with_capacity(Basic* basic, uint32_t capacity)
{
    String* str;
    
    if (capacity < MIN_CAPACITY)
        capacity = MIN_CAPACITY;
    else
        capacity = bit_pow2_greater_or_equal(capacity);
    
    str = eqp_alloc_type_bytes(basic, sizeof(String) + capacity, String);
    
    str->length     = 0;
    str->capacity   = capacity;
    str->refCount   = 1;
    
    return str;
}

static String* string_realloc_next_pow2(Basic* basic, String* str, uint32_t len)
{
    uint32_t cap    = bit_pow2_greater_than(len);
    size_t size     = cap + sizeof(String) + 1;
    str             = eqp_realloc_type_bytes(basic, str, size, String);
    
    str->length     = len;
    str->capacity   = cap;
    
    return str;
}

static String* string_realloc_and_fill(Basic* basic, String* str, const char* src, uint32_t len)
{
    str = string_realloc_next_pow2(basic, str, len);
    
    memcpy(str->data, src, len);
    str->data[len]  = 0;
    
    return str;
}

String* string_create_from_cstr(Basic* basic, const char* src, uint32_t len)
{
    String* str     = string_realloc_and_fill(basic, NULL, src, len);
    str->refCount   = 1;
    return str;
}

String* string_create_from_file(Basic* basic, FILE* fp)
{
    uint32_t len    = (uint32_t)file_calc_size(fp);
    String* str     = string_create_with_capacity(basic, len + 1); // Make sure there will be room for the guaranteed null terminator
    
    if (len > 0)
    {
        str->length = len;
        fread(str->data, sizeof(byte), len, fp);
        rewind(fp);
    }
    
    str->data[len] = 0;
    
    return str;
}

void string_grab(String* str)
{
    str->refCount++;
}

void string_drop(String* str)
{
    str->refCount--;
    
    if (str->refCount <= 0)
        free(str);
}

void string_clear(String* str)
{
    str->length     = 0;
    str->data[0]    = 0;
}

void string_set_from_cstr(Basic* basic, String** str, const char* src, uint32_t len)
{
    String* base = *str;
    
    if (len == 0 || src == NULL)
    {
        base->length = 0;
    }
    else if (len < base->capacity)
    {
        base->length = len;
        memcpy(base->data, src, len);
        base->data[len] = 0;
    }
    else
    {
        *str = string_realloc_and_fill(basic, base, src, len);
    }
}

void string_set_from_format(Basic* basic, String** str, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    string_set_from_vformat(basic, str, fmt, args);
    va_end(args);
}

void string_set_from_vformat(Basic* basic, String** str, const char* fmt, va_list args)
{
    String* base = *str;
    va_list args_copy;
    int len;
    
    va_copy(args_copy, args);
    
    len = vsnprintf(NULL, 0, fmt, args);
    
    if (len < 0)
        exception_throw(basic, ErrorFormatString);
    
    if (base->capacity <= (uint32_t)len)
    {
        uint32_t cap    = bit_pow2_greater_than((uint32_t)len);
        base            = eqp_realloc_type_bytes(basic, base, cap + sizeof(String) + 1, String);
        
        base->capacity  = cap;
        *str            = base;
    }
    
    len = vsnprintf(base->data, base->capacity, fmt, args_copy);
    
    if (len < 0)
        exception_throw(basic, ErrorFormatString);
    
    base->length = (uint32_t)len;
    
    va_end(args_copy);
}

const char* string_get_data(String* str)
{
    return (str->length == 0) ? NULL : str->data;
}

uint32_t string_get_length(String* str)
{
    return str->length;
}

uint32_t string_get_capacity(String* str)
{
    return str->capacity;
}

void string_add_char(Basic* basic, String** str, int c)
{
    String* base    = *str;
    uint32_t index  = base->length++;
    
    if (base->length >= base->capacity)
    {
        base = string_realloc_next_pow2(basic, base, base->length);
        *str = base;
    }
    
    base->data[index + 0] = (char)c;
    base->data[index + 1] = 0;
}

void string_add_cstr(Basic* basic, String** str, const char* cstr, uint32_t len)
{
    String* base    = *str;
    uint32_t index  = base->length;
    uint32_t end    = index + len;
    uint32_t i;
    
    if (end >= base->capacity)
    {
        base = string_realloc_next_pow2(basic, base, end);
        *str = base;
    }
    else
    {
        base->length = end;
    }
    
    for (i = 0; i < len; i++)
    {
        base->data[index++] = cstr[i];
    }
    
    base->data[end] = 0;
}

void string_add_string(Basic* basic, String** str, String* src)
{
    string_add_cstr(basic, str, src->data, src->length);
}

int string_compare_cstr(String* str, const char* cstr)
{
    return strcmp(str->data, cstr);
}

#undef MIN_CAPACITY
