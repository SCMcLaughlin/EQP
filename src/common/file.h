
#ifndef EQP_FILE_H
#define EQP_FILE_H

#include "define.h"
#include "eqp_core.h"
#include "eqp_alloc.h"
#include "eqp_string.h"
#include <zlib.h>

uint64_t    file_calc_size(FILE* fp);
uint64_t    file_calc_size_remaining(FILE* fp);
byte*       file_read_remaining(Basic* basic, FILE* fp, uint32_t* len);
byte*       file_read_remaining_decompress_no_throw(Basic* basic, FILE* fp, uint32_t* len, uint32_t inflatedLength);

#endif//EQP_FILE_H
