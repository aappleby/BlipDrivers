#ifndef MATH_H_
#define MATH_H_

#include <stdint.h>

// Xorshift random number generator. Kinda massive overkill for this
// application, but it's simple.
uint32_t xor128(void);

// Multiplies two signed 16-bit fractions.
int16_t  mul_s16 (int16_t a, int16_t b);

// Multiplies two unsigned 16-bit fractions.
uint16_t mul_u16 (uint16_t a, uint16_t b);

// Linearly interpolate between two numbers.
uint8_t  lerp_u8 (uint8_t a,  uint8_t b,  uint8_t x);
int8_t   lerp_s8 (int8_t a,   int8_t b,   uint8_t x);
uint16_t lerp_u16(uint16_t a, uint16_t b, uint8_t x);
int16_t  lerp_s16(int16_t a,  int16_t b,  uint8_t x);

// Linearly interpolate between elements in a look-up table.
// Roughly equivalent to "lerp(table[x>>8], table[(x>>8)+1], x & 0xFF);"
uint8_t  lerp_u8_u8  (const uint8_t* table, uint16_t x);

// Produces a 16-bit result for better precision.
uint16_t lerp_u8_u16 (const uint8_t* table, uint16_t x);

// Interpolates without wrapping around the end of the table.
// Requrires a _257_-element table.
uint16_t lerp_u8_u16_nowrap(const uint8_t* table, uint16_t x);
uint16_t lerp_u16_u16_nowrap(const uint16_t* table, uint16_t x);

// Same as above, but with a 16-bit table.
uint16_t lerp_u16_u16_nowrap (const uint16_t* table, uint16_t x);

// Interpolates between adjacent pixels in an image.
uint8_t imagelerp_u8(const uint8_t* image, uint16_t x);

#endif /* MATH_H_ */