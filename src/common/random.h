
#ifndef EQP_RANDOM_H
#define EQP_RANDOM_H

#include "define.h"
#include <sqlite3.h>

void        random_bytes(void* buffer, int count);
uint16_t    random_uint16();
uint32_t    random_uint32();
uint64_t    random_uint64();
float       random_float();
int         random_chance(float percent);

#endif//EQP_RANDOM_H
