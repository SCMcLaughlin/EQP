
#ifndef EQP_ALLOC_H
#define EQP_ALLOC_H

#include "define.h"
#include "exception.h"

STRUCT_DECLARE(Basic);

void*   eqp_realloc(R(Basic*) basic, R(void*) ptr, size_t len);
#define eqp_realloc_type_bytes(es, ptr, len, type) ((type*)eqp_realloc((es), (ptr), (len)))
#define eqp_realloc_type_array(es, ptr, num, type) eqp_realloc_type_bytes(es, ptr, sizeof(type) * (num), type)
#define eqp_realloc_type(es, ptr, type) eqp_realloc_type_bytes(es, ptr, sizeof(type), type)
#define eqp_alloc(es, len) eqp_realloc((es), NULL, (len))
#define eqp_alloc_type_bytes(es, len, type) ((type*)eqp_alloc(es, len))
#define eqp_alloc_type_array(es, num, type) eqp_alloc_type_bytes(es, sizeof(type) * (num), type)
#define eqp_alloc_type(es, type) eqp_alloc_type_bytes(es, sizeof(type), type)

#endif//EQP_ALLOC_H
