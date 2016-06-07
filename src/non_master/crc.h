
#ifndef EQP_CRC_H
#define EQP_CRC_H

#include "define.h"
#include "netcode.h"

uint16_t    crc_calc16(R(const void*) data, uint32_t len, uint32_t key);
uint16_t    crc_calc16_network(R(const void*) data, uint32_t len, uint32_t key);
uint32_t    crc_calc32(R(const void*) data, uint32_t len);
uint32_t    crc_calc32_network(R(const void*) data, uint32_t len);

#endif//EQP_CRC_H
