#include "Math.h"
#include "Tables.h"

#include <stdint.h>

#include <avr/pgmspace.h>

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Fractional multiply functions.

// Multiply two unsigned 16-bit fractions.
__attribute__((naked)) uint16_t mul_f16(uint16_t a, uint16_t b) {
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
  
  // ah * bl
  asm("mul r19, r20");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  // bh * al
  asm("mul r21, r18");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  asm("clr r1");
  asm("ret");
}

// Multiply two signed 16-bit fractions.
__attribute__((naked)) int16_t mul_f16(int16_t a, int16_t b) {
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
  
  // The result effectively has two sign bits, so shift
  // it left 1 bit.  
  asm("rol r23");
  asm("rol r24");
  asm("rol r25");
  
  asm("clr r1");
  asm("ret");
}

// Multiply one signed and one unsigned 16-bit fraction.
__attribute__((naked)) int16_t mul_f16(int16_t a, uint16_t b) {
  // r26 = 0
  asm("clr r26");

  // r19:r18 = a
  asm("movw r18, r24");

  // r21:r20 = b
  asm("movw r20, r22");
  
  // r25:r24:r23:r22 = scratch
  
  // scratch = ((signed)ah * bh) << 16;
  asm("mulsu r19, r21");
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
  
  // bh * al
  asm("mul r21, r18");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");

  asm("clr r1");
  asm("ret");
}  


// Multiply one unsigned and one signed 16-bit fraction.
__attribute__((naked)) int16_t mul_f16(uint16_t a, int16_t b) {
  // r26 = 0
  asm("clr r26");

  // r19:r18 = a
  asm("movw r18, r24");

  // r21:r20 = b
  asm("movw r20, r22");
  
  // r25:r24:r23:r22 = scratch
  // scratch = (ah * (signed)bh) << 16;
  asm("mulsu r21, r19");
  asm("movw r24, r0");
  
  // scratch += al*bl
  asm("mul r18, r20");
  asm("movw r22, r0");
  
  // ah * bl
  asm("mul r19, r20");
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

//-----------------------------------------------------------------------------
// Interpolate between two values.

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

//-----------------------------------------------------------------------------
// Interpolated table lookup.

// Interpolate between elements in a 256-element, 8-bit table in flash, with
// wrapping.
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


// Interpolate between elements in a 256-element, 8-bit table in flash, with
// wrapping, expanding the table value out to 16 bits.
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


// Interpolate between elements in a 256-element, 8-bit table in RAM,
// with wrapping, expanding the table value out to 16 bits.
__attribute__((naked)) uint16_t lerp_u8_u16_ram(uint8_t* table, uint16_t x) {
  asm("clr r20");
  asm("clr r21");
  
  // y1 = table[(x >> 8)]
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("ld r26, z");

  // x1++;
  asm("inc r23");
  
  // y2 = table[(x >> 8) + 1];
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("ld r27, z");
  
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

// Interpolate between elements in a 256-element, signed 8-bit table, with wrapping,
// expanding the table values out to a signed 16-bit value.
// input r25:r24 - table
// input r23:r22 - x
// output r25:r24 - signed 16-bit interpolated table value.
__attribute__((naked)) int16_t lerp_s8_s16(const int8_t* table, uint16_t x) {
  // r26 = 0.
  asm("clr r26");
  
  // r19:r18 = y1 = expand(table[(x >> 8)])
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r18, z");
  asm("mov r19, r18");
  asm("sbrc r18, 7");
  asm("subi r19, 1");

  // x1++;
  asm("inc r23");
  
  // r21:r20 = y2 = expand(table[(x >> 8) + 1]);
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r20, z");
  asm("mov r21, r20");
  asm("sbrc r20, 7");
  asm("subi r21, 1");
  
  // r22 = t = interpolation factor
  // r25:r24:r23 = scratch
  
  // scratch = ((signed)y2h * t) << 8
  asm("mulsu r21, r22");
  asm("clr r23");
  asm("mov r24, r0");
  asm("mov r25, r1");
  
  // scratch += y2l * t
  asm("mul r20, r22");
  asm("mov r23, r0");
  asm("add r24, r1");
  asm("adc r25, r26");
  
  // t = ~t
  asm("com r22");
  
  // scratch += ((signed)y1h * t) << 8
  asm("mulsu r19, r22");
  asm("add r24, r0");
  asm("adc r25, r1");
  
  // scratch += y1l * t
  asm("mul r18, r22");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  // scratch += y1
  asm("add r23, r18");
  asm("adc r24, r19");
  asm("adc r25, r26");
  
  asm("clr r1");
  asm("ret");
}


int16_t lerp_s8_s16(const int8_t* table, int16_t x) {
  return lerp_s8_s16(table, uint16_t(x ^ 0x8000));
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


uint8_t lerp_u8_u8_nowrap(const uint8_t* table, uint16_t x) {
  return lerp_u8_u16_nowrap(table, x) >> 8;
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

//-----------------------------------------------------------------------------
// Color class

uint8_t hex2dec(char code) {
  uint8_t x = code - 48;
  if(x > 9) x -= 7;
  if(x > 15) x -= 32;
  return x;
}  

Color::Color(const char* hexcode) {
  uint8_t r, g, b;
  
  if (hexcode[3] == 0) {
    // rgb
    r = hex2dec(hexcode[0]);
    g = hex2dec(hexcode[1]);
    b = hex2dec(hexcode[2]);
    r |= r << 4;
    g |= g << 4;
    b |= b << 4;
  }
  else if (hexcode[4] == 0) {
    // #rgb
    r = hex2dec(hexcode[1]);
    g = hex2dec(hexcode[2]);
    b = hex2dec(hexcode[3]);
    r |= r << 4;
    g |= g << 4;
    b |= b << 4;
  }    
  else if(hexcode[0] == '#') {
    // #rrggbb
    r = hex2dec(hexcode[1]) * 16 + hex2dec(hexcode[2]);
    g = hex2dec(hexcode[3]) * 16 + hex2dec(hexcode[4]);
    b = hex2dec(hexcode[5]) * 16 + hex2dec(hexcode[6]);
  }
  else {
    // rrggbb
    r = hex2dec(hexcode[0]) * 16 + hex2dec(hexcode[1]);
    g = hex2dec(hexcode[2]) * 16 + hex2dec(hexcode[3]);
    b = hex2dec(hexcode[4]) * 16 + hex2dec(hexcode[5]);
  }
  
  this->r = r | r << 8;
  this->g = g | g << 8;
  this->b = b | b << 8;
}  

Color Color::fromHue(uint16_t h) {
  Color c;
  c.r = blip_hsv_r(h);
  c.g = blip_hsv_g(h);
  c.b = blip_hsv_b(h);
  return c;
}

Color Color::fromHue(float h) {
  return fromHue(uint16_t(h * 65536));
}
  
Color Color::fromHue(double h) {
  return fromHue(uint16_t(float(h) * 65536));
}

Color Color::fromHex(const char* code) {
  return Color(code);
}  

//-----------------------------------------------------------------------------
// Bliplace API, scalar functions.

uint16_t blip_sin(uint16_t x) {
  return lerp_u8_u16(sintab, x);
}

uint16_t blip_cos(uint16_t x) {
  return lerp_u8_u16(sintab, x + 16384);
}

int16_t blip_ssin(uint16_t x) {
  return blip_sin(x) ^ 0x8000;
}  

int16_t blip_scos(uint16_t x) {
  return blip_cos(x) ^ 0x8000;
}  

uint16_t blip_scale(uint16_t x, uint16_t s) {
  return mul_f16(x, s);
}

int16_t blip_scale(int16_t x, uint16_t s) {
  return mul_f16(x, s);
}  

// 'Smooth' add - 
uint16_t blip_smadd(uint16_t a, uint16_t b) {
  return a + b - mul_f16(a, b);
}  

uint16_t blip_hsv_r(uint16_t h) {
  return lerp_u8_u16(huetab, h);
}

uint16_t blip_hsv_g(uint16_t h) {
  return lerp_u8_u16(huetab, h + 21845);
}

uint16_t blip_hsv_b(uint16_t h) {
  return lerp_u8_u16(huetab, h + 43690);
}

uint16_t blip_pow2(uint16_t x) {
  return blip_scale(x, x);
}

uint16_t blip_pow3(uint16_t x) {
  uint16_t x2 = blip_scale(x, x);
  return blip_scale(x, x2);
}
  
uint16_t blip_pow4(uint16_t x) {
  uint16_t x2 = blip_scale(x, x);
  return blip_scale(x2, x2);
}
  
uint16_t blip_pow5(uint16_t x) {
  uint16_t x2 = blip_scale(x, x);
  uint16_t x3 = blip_scale(x, x2);
  return blip_scale(x2, x3);
}  

uint16_t blip_pow6(uint16_t x) {
  uint16_t x2 = blip_scale(x, x);
  uint16_t x3 = blip_scale(x, x2);
  return blip_scale(x3, x3);
}  

uint16_t blip_pow7(uint16_t x) {
  uint16_t x2 = blip_scale(x, x);
  uint16_t x3 = blip_scale(x, x2);
  uint16_t x4 = blip_scale(x2, x2);
  return blip_scale(x3, x4);
}  

uint16_t blip_noise(uint16_t x) {
  return lerp_u8_u16(noise, x);
}

uint16_t blip_lookup(const uint8_t* table, uint16_t x) {
  return lerp_u8_u16(table, x);
}

uint16_t blip_random() {
  return (uint16_t)xor128();
}

uint16_t blip_history1(uint16_t x) {
  uint16_t cursor = blip_tick;
  cursor -= x;
  return lerp_u8_u16_ram(blip_history, cursor);
}

uint16_t blip_history2(uint16_t x) {
  uint16_t cursor = blip_tick;
  cursor -= x;
  return lerp_u8_u16_ram(blip_history + 256, cursor);
}

//-----------------------------------------------------------------------------
// Bliplace API, color functions.

Color blip_scale(Color const& c, uint16_t s) {
  return Color(blip_scale(c.r, s),
               blip_scale(c.g, s),
               blip_scale(c.b, s));
}


Color blip_scale(Color const& c, uint16_t s1, uint16_t s2) {
  return Color(blip_scale(blip_scale(c.r, s1), s2),
               blip_scale(blip_scale(c.g, s1), s2),
               blip_scale(blip_scale(c.b, s1), s2));
}


Color operator + (Color const& a, Color const& b) {
  return Color(a.r + b.r, a.g + b.g, a.b + b.b);
}

Color blip_smadd(Color const& a, Color const& b) {
  return Color(blip_smadd(a.r, b.r),
               blip_smadd(a.g, b.g),
               blip_smadd(a.b, b.b));
}

Color blip_lerp(Color const& a, Color const& b, uint16_t x) {
  uint8_t t = x >> 8;
  return Color(lerp_u16(a.r, b.r, t),
               lerp_u16(a.g, b.g, t),
               lerp_u16(a.b, b.b, t));
}
