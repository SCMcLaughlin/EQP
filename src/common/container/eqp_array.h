
#ifndef EQP_ARRAY_H
#define EQP_ARRAY_H

#include "define.h"

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(Array);

Array*      array_create(Basic* basic, size_t elementSize);
#define     array_create_type(basic, type) array_create((basic), sizeof(type))
Array*      array_create_with_capacity(Basic* basic, size_t elementSize, uint32_t capacity);
#define     array_create_with_capacity_type(basic, type, capacity) array_create_with_capacity((basic), sizeof(type), (capacity))
#define     array_destroy(array) free(array)

uint32_t    array_count(Array* array);
uint32_t    array_element_size(Array* array);
void*       array_data(Array* array);
#define     array_data_type(array, type) ((type*)array_data((array)))
#define     array_empty(array) (array_count((array)) == 0)

void*       array_get(Array* array, uint32_t index);
#define     array_get_type(array, index, type) ((type*)array_get((array), (index)))
void*       array_back(Array* array);
#define     array_back_type(array, type) ((type*)array_back((array)))
void        array_get_copy(Array* array, uint32_t index, void* copyTo);
void        array_back_copy(Array* array, void* copyTo);
void        array_set(Array* array, uint32_t index, const void* value);
#define     array_push_back_type(basic, array, type) ((type*)array_push_back((basic), (array), NULL))
void*       array_push_back(Basic* basic, Array** array, const void* value);
void        array_pop_back(Array* array);
int         array_swap_and_pop(Array* array, uint32_t index);
void        array_shift_left(Array* array, uint32_t numIndices);

void        array_reserve(Basic* basic, Array** array, uint32_t count);
void        array_clear(Array* array);

#endif//EQP_ARRAY_H
