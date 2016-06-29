
#ifndef EQP_STRING_H
#define EQP_STRING_H

#include "define.h"
#include "bit.h"

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(String);

String*     string_create(Basic* basic);
String*     string_create_with_capacity(Basic* basic, uint32_t capacity);
String*     string_create_from_cstr(Basic* basic, const char* str, uint32_t len);
String*     string_create_from_file(Basic* basic, FILE* fp);
#define     string_destroy(str) free(str)
String*     string_copy(Basic* basic, String* str);

void        string_clear(String* str);

void        string_set_from_cstr(Basic* basic, String** str, const char* src, uint32_t len);
void        string_set_from_format(Basic* basic, String** str, const char* fmt, ...);
void        string_set_from_vformat(Basic* basic, String** str, const char* fmt, va_list args);

const char* string_get_data(String* str);
uint32_t    string_get_length(String* str);
#define     string_data(str) string_get_data(str)
#define     string_length(str) string_get_length(str)

void        string_add_char(Basic* basic, String** str, int c);
void        string_add_cstr(Basic* basic, String** str, const char* cstr, uint32_t len);
void        string_add_string(Basic* basic, String** str, String* src);

int         string_compare_cstr(String* str, const char* cstr);

#endif//EQP_STRING_H
