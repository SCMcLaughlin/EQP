
#ifndef EQP_ARRAY_H
#define EQP_ARRAY_H

#include "define.h"

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(Array);

Array*      array_create(R(Basic*) basic, size_t elementSize);
#define     array_create_type(basic, type) array_create((basic), sizeof(type))
Array*      array_create_with_capacity(R(Basic*) basic, size_t elementSize, uint32_t capacity);
#define     array_create_with_capacity_type(basic, type, capacity) array_create_with_capacity((basic), sizeof(type), (capacity))
#define     array_destroy(array) free(array)

uint32_t    array_count(R(Array*) array);
uint32_t    array_element_size(R(Array*) array);
void*       array_data(R(Array*) array);
#define     array_data_type(array, type) (type*)array_data((array))
#define     array_empty(array) (array_count((array)) == 0)

void*       array_get(R(Array*) array, uint32_t index);
#define     array_get_type(array, index, type) (type*)array_get((array), (index))
void*       array_back(R(Array*));
void        array_get_copy(R(Array*) array, uint32_t index, void* copyTo);
void        array_back_copy(R(Array*) array, void* copyTo);
void        array_set(R(Array*) array, uint32_t index, void* value);
void*       array_push_back(R(Basic*) basic, R(Array**) array, void* value);
void        array_pop_back(R(Array*) array);
void        array_swap_and_pop(R(Array*) array, uint32_t index);

void        array_reserve(R(Basic*) basic, R(Array**) array, uint32_t count);
void        array_clear(R(Array*) array);

#endif//EQP_ARRAY_H
