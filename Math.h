#ifndef MATH_H_
#define MATH_H_

#include "LEDDriver.h"

#include <stdint.h>

struct Color;

uint16_t blip_sin(uint16_t x);
uint16_t blip_cos(uint16_t x);
int16_t  blip_ssin(uint16_t x);
int16_t  blip_scos(uint16_t x);
uint16_t blip_hsv_r(uint16_t h);
uint16_t blip_hsv_g(uint16_t h);
uint16_t blip_hsv_b(uint16_t h);
uint16_t blip_pow2(uint16_t x);
uint16_t blip_pow3(uint16_t x);
uint16_t blip_pow4(uint16_t x);
uint16_t blip_pow5(uint16_t x);
uint16_t blip_pow6(uint16_t x);
uint16_t blip_pow7(uint16_t x);
uint16_t blip_noise(uint16_t x);
uint16_t blip_scale(uint16_t x, uint16_t s);
uint16_t blip_smadd(uint16_t a, uint16_t b);
int16_t  blip_scale(int16_t x, uint16_t s);
uint16_t blip_lookup(const uint8_t* table, uint16_t x);
uint16_t blip_random();
uint16_t blip_history1(uint16_t x);
uint16_t blip_history2(uint16_t x);

Color    blip_scale(Color const& c, uint16_t s);
Color    blip_scale(Color const& c, uint16_t s1, uint16_t s2);
Color    blip_smadd(Color const& a, Color const& b);
Color    blip_lerp(Color const& a, Color const& b, uint16_t x);
Color    blip_hsv(uint16_t h);

struct Color : public Pixel {
  Color() {};
    
  Color(const char* hexcode);
  
  Color(uint16_t r1, uint16_t g1, uint16_t b1)
  {
    r = r1;
    g = g1;
    b = b1;
  }
  
  Color(float r1, float g1, float b1)
  {
    r = r1 * 65535;
    g = g1 * 65535;
    b = b1 * 65535;
  }    

  Color(double r1, double g1, double b1)
  {
    r = float(r1) * 65535;
    g = float(g1) * 65535;
    b = float(b1) * 65535;
  }    

  Color(int r1, int g1, int b1)
  {
    r = r1;
    g = g1;
    b = b1;
  }

  Color(uint8_t r1, uint8_t g1, uint8_t b1)
  {
    r = (r1 << 8) | r1;
    g = (g1 << 8) | g1;
    b = (b1 << 8) | b1;
  }
  
  Color(Pixel const& p) {
    r = p.r;
    g = p.g;
    b = p.b;
  }
  
  operator Pixel () const {
    Pixel t;
    t.r = r;
    t.g = g;
    t.b = b;
    return t;
  }
  
  static Color fromHue(uint16_t h);
  static Color fromHue(float h);
  static Color fromHue(double h);
  static Color fromHex(const char* hexcode);
  
  Color operator + (Color const& x) const {
    return Color(r + x.r, g + x.g, b + x.b);
  }
};  

/*
// Xorshift random number generator. Kinda massive overkill for this
// application, but it's simple.
uint32_t xor128(void);

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

uint16_t lerp_u8_u16_ram(uint8_t* table, uint16_t x);

int16_t lerp_s8_s16(const int8_t* table, uint16_t x);
int16_t lerp_s8_s16(const int8_t* table, int16_t x);

// Interpolates without wrapping around the end of the table.
// Requrires a _257_-element table.
uint8_t lerp_u8_u8_nowrap (const uint8_t* table, uint16_t x);
uint16_t lerp_u8_u16_nowrap(const uint8_t* table, uint16_t x);
uint16_t lerp_u16_u16_nowrap(const uint16_t* table, uint16_t x);

// Same as above, but with a 16-bit table.
uint16_t lerp_u16_u16_nowrap (const uint16_t* table, uint16_t x);

// Interpolates between adjacent pixels in an image.
uint8_t imagelerp_u8(const uint8_t* image, uint16_t x);
*/

#endif /* MATH_H_ */