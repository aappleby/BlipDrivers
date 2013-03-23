#ifndef MATH_H_
#define MATH_H_

#include <stdint.h>

uint32_t xor128(void);

uint8_t tablelerp8_asm(const uint8_t* table, uint16_t x);
uint16_t tablelerp8_asm2(const uint8_t* table, uint16_t x);
uint8_t lerp8_asm(uint8_t x1, uint8_t x2, uint8_t t);
uint16_t lerp16_asm(uint16_t x1, uint16_t x2, uint8_t t);

#endif /* MATH_H_ */