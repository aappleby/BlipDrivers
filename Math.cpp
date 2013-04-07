#include "Math.h"
#include <stdint.h>

#include <avr/pgmspace.h>

struct fixed {
  fixed() {};
  fixed(int16_t v2) { v = v2; };
  fixed& operator = (int16_t v2) { v = v2; return *this; }
  fixed& operator = (const fixed& v2) { v = v2.v; return *this; }
  fixed& operator = (volatile fixed v2) { v = v2.v; return *this; }
  __attribute__((noinline)) fixed operator * (fixed b) {
    return int32_t(v) * int32_t(b.v) >> 12;
  }
  operator int16_t() const { return v; }
  int16_t v;
};  


// Xorshift RNG, from Wikipedia.
uint32_t xor128(void) {
  static uint32_t x = 123456789;
  static uint32_t y = 362436069;
  static uint32_t z = 521288629;
  static uint32_t w = 88675123;
  uint32_t t;
 
  t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
}

// Multiply two signed 16-bit fractions.
__attribute__((naked)) int16_t mul_s16(int16_t a, int16_t b) {
  // r26 = 0
  asm("clr r26");

  // r19:r18 = a
  asm("movw r18, r24");

  // r21:r20 = b
  asm("movw r20, r22");
  
  // r25:r24:r23:r22 = scratch
  // scratch = ((signed)ah * (signed)bh) << 16;
  asm("muls r19, r21");
  asm("movw r24, r0");
  
  // scratch += al*bl
  asm("mul r18, r20");
  asm("movw r22, r0");
  
  // (signed)ah * bl
  asm("mulsu r19, r20");
  asm("sbc r25, r26");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  // (signed)bh * al
  asm("mulsu r21, r18");
  asm("sbc r25, r26");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  asm("clr r1");
  asm("ret");
}  

// Multiply two unsigned 16-bit fractions.
__attribute__((naked)) uint16_t mul_u16(uint16_t a, uint16_t b) {
  asm("clr r26");
  
  // r19:r18 = a
  asm("movw r18, r24");

  // r21:r20 = b  
  asm("movw r20, r22");
  
  // r25:r24:r23:r22 = scratch
  // scratch = ((signed)ah * (signed)bh) << 16;
  asm("mul r19, r21");
  asm("movw r24, r0");
  
  // scratch += al*bl
  asm("mul r18, r20");
  asm("movw r22, r0");
  
  // (signed)ah * bl
  asm("mul r19, r20");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  // (signed)bh * al
  asm("mul r21, r18");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  asm("clr r1");
  asm("ret");
}

// Interpolate between two unsigned 8-bit numbers.
__attribute__((naked)) uint8_t lerp_u8(uint8_t x1, uint8_t x2, uint8_t t) {
  // r24:r23 is our scratch space.
  asm("mov r21, r24"); // x1 is now in r21
  
  // scratch = x2 * t
  asm("mul r22, r20");
  asm("mov r23, r0");
  asm("mov r24, r1");
  
  // t = ~t
  asm("com r20");
  
  // scratch += x1 * ~t
  asm("mul r21, r20");
  asm("add r23, r0");
  asm("adc r24, r1");

  asm("clr r1");  
  
  // scratch += x1
  asm("add r23, r21");
  asm("adc r24, r1");

  asm("ret");
}  

// Interpolate between two signed 8-bit numbers.
__attribute__((naked)) int8_t lerp_s8(int8_t a, int8_t b, uint8_t t) {
  return lerp_u8(a ^ 0x80, b ^ 0x80, t) ^ 0x80;
}  

// Interpolate between two unsigned 16-bit numbers.
__attribute__((naked)) uint16_t lerp_u16(uint16_t x1, uint16_t x2, uint8_t t) {
  // r21 = 0
  // r20:r19:r18 is our scratch space.
  asm("mov r26, r20");
  asm("clr r20");
  asm("clr r21");
  
  // scratch = x2l * t
  asm("mul r22, r26");
  asm("movw r18, r0");
  
  // scratch += (x2h * t) << 8;
  asm("mul r23, r26");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // t = ~t
  asm("com r26");
  
  // scratch += x1l * ~t
  asm("mul r24, r26");
  asm("add r18, r0");
  asm("adc r19, r1");
  asm("adc r20, r21");
  
  // scratch += (x1h * ~t) << 8;
  asm("mul r25, r26");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // scratch += x1
  asm("add r18, r24");
  asm("adc r19, r25");
  asm("adc r20, r21");
  
  // result = scratch >> 8
  asm("mov r24, r19");
  asm("mov r25, r20");
  
  asm("clr r1");
  asm("ret");
}

// Interpolate between two signed 16-bit numbers.
__attribute__((naked)) uint16_t lerp16_s16(int16_t a, int16_t b, uint8_t t) {
  return lerp_u16(a ^ 0x8000, b ^ 0x8000, t) ^ 0x8000;
}  

// Interpolate between elements in a 256-element, 8-bit table, with wrapping.
__attribute__((naked)) uint8_t lerp_u8_u8(const uint8_t* table, uint16_t x) {
  // x1 = x & 0xff; (in r23)
  // t = x >> 8;  (in r22);
  
  // y1 = table[x1] (in r20);
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r20, z");

  // x1++;
  asm("inc r23");
  
  // y2 = table[x1]; (in r21);
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r21, z");
  
  // scratch = y2 * t
  asm("mul r21, r22");
  asm("mov r23, r0");  
  asm("mov r24, r1");
  
  // t = ~t
  asm("com r22");
  
  // scratch += y1 * ~t
  asm("mul r20, r22");
  asm("add r23, r0");
  asm("adc r24, r1");
  
  asm("clr r1");
  
  // scratch += y1
  asm("add r23, r20");
  asm("adc r24, r1");
  
  asm("ret");
}  


// Interpolate between elements in a 256-element, 8-bit table, with wrapping,
// expanding the table value out to 16 bits.
__attribute__((naked)) uint16_t lerp_u8_u16(const uint8_t* table, uint16_t x) {
  asm("clr r20");
  asm("clr r21");
  
  // y1 = table[(x >> 8)]
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r26, z");

  // x1++;
  asm("inc r23");
  
  // y2 = table[(x >> 8) + 1];
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r27, z");
  
  // scratch = (y2 * 257) * t
  asm("mul r27, r22");
  asm("movw r18, r0");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // t = ~t
  asm("com r22");
  
  // scratch += (y1 * 257) * ~t
  asm("mul r26, r22");
  asm("add r18, r0");
  asm("adc r19, r1");
  asm("adc r20, r21");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // scratch += (y1 * 257)
  asm("add r18, r26");
  asm("adc r19, r26");
  asm("adc r20, r21");
  
  // result = scratch >> 8
  asm("mov r24, r19");
  asm("mov r25, r20");
  
  asm("clr r1");
  asm("ret");
}  

// Interpolate between elements in a 256-element, 8-bit table, no wrapping,
// expanding the table value out to 16 bits.
__attribute__((naked)) uint16_t lerp_u8_u16_nowrap(const uint8_t* table, uint16_t x) {
  asm("clr r20");
  asm("clr r21");
  
  // y1 = table[(x >> 8)]
  // y2 = table[(x >> 8) + 1];
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r26, z+");
  asm("lpm r27, z");

  // scratch = (y2 * 257) * t
  asm("mul r27, r22");
  asm("movw r18, r0");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // t = ~t
  asm("com r22");
  
  // scratch += (y1 * 257) * ~t
  asm("mul r26, r22");
  asm("add r18, r0");
  asm("adc r19, r1");
  asm("adc r20, r21");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // scratch += (y1 * 257)
  asm("add r18, r26");
  asm("adc r19, r26");
  asm("adc r20, r21");
  
  // result = scratch >> 8
  asm("mov r24, r19");
  asm("mov r25, r20");
  
  asm("clr r1");
  asm("ret");
}


// Interpolate between elements in a 16-bit table, no wrapping.
__attribute__((naked)) uint16_t lerp_u16_u16_nowrap(const uint16_t* table, uint16_t x) {
  // scratch = 0;
  asm("clr r20");
  asm("clr r21");
  
  // table += (x >> 8) * 2
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("add r30, r23");
  asm("adc r31, r1");

  // y1 = table[0];
  // y2 = table[1];
  asm("lpm r24, z+");
  asm("lpm r25, z+");
  asm("lpm r26, z+");
  asm("lpm r27, z");

  // scratch = y2.l * t
  asm("mul r26, r22");
  asm("movw r18, r0");
  
  // scratch += (y2.h * t) << 8;
  asm("mul r27, r22");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // t = ~t
  asm("com r22");
  
  // scratch += y1.l * ~t
  asm("mul r24, r22");
  asm("add r18, r0");
  asm("adc r19, r1");
  asm("adc r20, r21");
  
  // scratch += (y1.h * ~t) << 8;
  asm("mul r25, r22");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // scratch += y1
  asm("add r18, r24");
  asm("adc r19, r25");
  asm("adc r20, r21");
  
  // result = scratch >> 8
  asm("mov r24, r19");
  asm("mov r25, r20");
  
  asm("clr r1");
  asm("ret");
}  

// Interpolate between two values in an 8-bit array that are 24 bytes apart
uint8_t imagelerp_u8(const uint8_t* image, uint16_t x) {
  uint8_t s = x;
  uint8_t t = ~s;
  
  uint8_t x1 = x >> 8;
  
  const uint8_t* p = image + (x1 * 24);
  uint16_t a = pgm_read_byte(p);
  p += 24;
  uint16_t b = pgm_read_byte(p);
  
  return ((b * s) + (a * t) + a) >> 8;
}
