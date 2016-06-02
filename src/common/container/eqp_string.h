
#ifndef EQP_STRING_H
#define EQP_STRING_H

#include "define.h"
#include "bit.h"

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(String);

String*     string_create(R(Basic*) basic);
String*     string_create_with_capacity(R(Basic*) basic, uint32_t capacity);
String*     string_create_from_cstr(R(Basic*) basic, R(const char*) str, uint32_t len);
String*     string_create_from_file(R(Basic*) basic, R(FILE*) fp);
#define     string_destroy(str) free(str)
String*     string_copy(R(Basic*) basic, R(String*) str);

void        string_clear(R(String*) str);

void        string_set_from_cstr(R(Basic*) basic, R(String**) str, R(const char*) src, uint32_t len);
void        string_set_from_format(R(Basic*) basic, R(String**) str, R(const char*) fmt, ...);
void        string_set_from_vformat(R(Basic*) basic, R(String**) str, R(const char*) fmt, va_list args);

const char* string_get_data(R(String*) str);
uint32_t    string_get_length(R(String*) str);
#define     string_data(str) string_get_data(str)
#define     string_length(str) string_get_length(str)

void        string_add_char(R(Basic*) basic, R(String**) str, int c);
void        string_add_cstr(R(Basic*) basic, R(String**) str, R(const char*) cstr, uint32_t len);
void        string_add_string(R(Basic*) basic, R(String**) str, R(String*) src);

#endif//EQP_STRING_H
