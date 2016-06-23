
#ifndef EQP_HASH_TABLE_H
#define EQP_HASH_TABLE_H

#include "define.h"
#include "eqp_string.h"

/*
    String-keyed hash table, primarily for looking up Clients by name (for tells, etc)
*/

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(HashTable);

HashTable*  hash_table_create(R(Basic*) basic, size_t elementSize);
#define     hash_table_create_type(basic, type) hash_table_create((basic), sizeof(type))
void        hash_table_destroy(R(HashTable*) tbl);

// If the table already has an entry with the given key, it will not be overwritten and these will return false
int         hash_table_set_by_cstr(R(Basic*) basic, R(HashTable**) tbl, R(const char*) key, uint32_t length, R(void*) value);
#define     hash_table_set_by_str(basic, tbl, str, value) hash_table_set_by_cstr((basic), (tbl), string_data((str)), string_length((str)), (value))

void*       hash_table_get_by_cstr(R(HashTable*) tbl, R(const char*) key, uint32_t length);
#define     hash_table_get_by_str(tbl, str) hash_table_get_by_cstr((tbl), string_data((str)), string_length((str)))
#define     hash_table_get_type_by_cstr(tbl, key, len, type) (type*)hash_table_get_by_cstr((tbl), (key), (len))
#define     hash_table_get_type_by_str(tbl, str, type) hash_table_get_type_by_cstr(tbl, string_data((str)), string_length((str)), type)

void        hash_table_remove_by_cstr(R(HashTable*) tbl, R(const char*) key, uint32_t length);
#define     hash_table_remove_by_str(tbl, str) hash_table_remove_by_cstr((tbl), string_data((str)), string_length((str)))

#endif//EQP_HASH_TABLE_H
