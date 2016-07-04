
#include "eqp_array.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"

#define MIN_CAPACITY 2

STRUCT_DEFINE(Array)
{
    uint32_t    count;
    uint32_t    capacity;
    uint32_t    elementSize;
    uint32_t    padding;
    byte        data[0];    // Aligned on a 16-byte boundary
};

Array* array_create(Basic* basic, size_t elementSize)
{
    return array_create_with_capacity(basic, elementSize, MIN_CAPACITY);
}

Array* array_create_with_capacity(Basic* basic, size_t elementSize, uint32_t capacity)
{
    Array* array;
    
    if (capacity < MIN_CAPACITY)
        capacity = MIN_CAPACITY;
    
    array = eqp_alloc_type_bytes(basic, sizeof(Array) + elementSize * capacity, Array);
    
    array->count        = 0;
    array->capacity     = capacity;
    array->elementSize  = elementSize;
    
    return array;
}

uint32_t array_count(Array* array)
{
    return array->count;
}

uint32_t array_element_size(Array* array)
{
    return array->elementSize;
}

void* array_data(Array* array)
{
    return array->data;
}

void* array_get(Array* array, uint32_t index)
{
    return (array->count > index) ? &array->data[index * array->elementSize] : NULL;
}

void* array_back(Array* array)
{
    uint32_t c = array->count;
    return (c > 0) ? &array->data[(c - 1) * array->elementSize] : NULL;
}

void array_get_copy(Array* array, uint32_t index, void* copyTo)
{
    if (array->count > index)
    {
        uint32_t size = array->elementSize;
        memcpy(copyTo, &array->data[index * size], size);
    }
}

void array_back_copy(Array* array, void* copyTo)
{
    uint32_t count  = array->count;
    uint32_t size   = array->elementSize;
    
    if (count > 0)
        memcpy(copyTo, &array->data[(count - 1) * size], size);
}

void array_set(Array* array, uint32_t index, const void* value)
{
    uint32_t size = array->elementSize;
    memcpy(&array->data[index * size], value, size);
}

void* array_push_back(Basic* basic, Array** array, const void* value)
{
    Array* ar       = *array;
    uint32_t index  = ar->count++;
    uint32_t cap    = ar->capacity;
    uint32_t size   = ar->elementSize;
    void* ptr;
    
    if (index >= cap)
    {
        cap *= 2;
        ar              = eqp_realloc_type_bytes(basic, ar, sizeof(Array) + cap * size, Array);
        ar->capacity    = cap;
        *array          = ar;
    }
    
    ptr = &ar->data[index * size];
    
    if (value)
        memcpy(ptr, value, size);
    
    return ptr;
}

void array_pop_back(Array* array)
{
    if (array->count > 0)
        array->count--;
}

int array_swap_and_pop(Array* array, uint32_t index)
{
    if (array->count > 0 && index < array->count)
    {
        uint32_t back = --array->count;
        
        if (index != back)
        {
            uint32_t size = array->elementSize;
            memcpy(&array->data[index * size], &array->data[back * size], size);
            return true;
        }
    }
    
    return false;
}

void array_shift_left(Array* array, uint32_t numIndices)
{
    uint32_t count = array_count(array);
    uint32_t size;
    
    if (count <= numIndices)
    {
        array_clear(array);
        return;
    }
    
    count -= numIndices;
    size = array->elementSize;
    
    memmove(array->data, &array->data[numIndices * size], count * size);
    array->count = count;
}

void array_reserve(Basic* basic, Array** array, uint32_t count)
{
    Array* ar = *array;
    
    if (ar->capacity >= count)
        return;
    
    ar->capacity    = count;
    *array          = eqp_realloc_type_bytes(basic, ar, sizeof(Array) + count * ar->elementSize, Array);
}

void array_clear(Array* array)
{
    array->count = 0;
}

void array_clear_from_index(Array* array, uint32_t index)
{
    if (index < array->count)
        array->count = index;
}

#undef MIN_CAPACITY
