#ifndef MATH_H_
#define MATH_H_

#include <stdint.h>

uint32_t xor128(void);

uint8_t tablelerp8_asm(const uint8_t* table, uint16_t x);
uint16_t tablelerp_asm2(const uint8_t* table, uint16_t x);
uint16_t tablelerp_asm3(const uint8_t* table, uint16_t x);

// requrires a 257-element table
uint16_t tablelerp_asm3_nowrap(const uint8_t* table, uint16_t x);
uint8_t lerp8_asm(uint8_t x1, uint8_t x2, uint8_t t);
uint16_t lerp16_asm(uint16_t x1, uint16_t x2, uint8_t t);

uint16_t tablelerp16_asm3_nowrap(const uint16_t* table, uint16_t x);

uint8_t imagelerp_u8(const uint8_t* image, uint16_t x);

#endif /* MATH_H_ */