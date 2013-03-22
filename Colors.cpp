#include "Colors.h"
#include "Tables.h"

#include <avr/pgmspace.h>

/*
uint8_t add_smooth(uint8_t a, uint8_t b) {
	return (((a + b) << 8) - (a*b)) >> 8;
}
*/

__attribute__((naked)) uint8_t add_smooth( uint8_t a, uint8_t b) {
	asm("mul r22, r24");
	asm("add r24, r22");
	asm("clr r22");
	asm("sub r22, r0");
	asm("sbc r24, r1");
	asm("clr r1");
	asm("ret");
};

void hue_to_rgb2(uint32_t hue, uint8_t& r, uint8_t& g, uint8_t& b)
{
  int segment = (hue >> 8) % 6;
  int phase = (hue & 0xFF);
  
  uint8_t up = phase;
  uint8_t down = 255 - phase;
  
  if(segment == 0)
  {
    // red -> yellow
    r = 255;
    g = up;
    b = 0;
  }
  else if (segment == 1)
  {
    // yellow -> green
    r = down;
    g = 255;
    b = 0;
  }
  else if (segment == 2)
  {
    // green ->cyan
    r = 0;
    g = 255;
    b = up;
  }
  else if (segment == 3)
  {
    // cyan -> blue
    r = 0;
    g = down;
    b = 255;
  }
  else if (segment == 4)
  {
    // blue -> magenta
    r = up;
    g = 0;
    b = 255;
  }
  else
  {
    // magenta -> red
    r = 255;
    g = 0;
    b = down;
  }
  
  r = pgm_read_byte(cielum + r);
  g = pgm_read_byte(cielum + g);
  b = pgm_read_byte(cielum + b);
  //r = cielum[r];
  //g = cielum[g];
  //b = cielum[b];
}


inline uint8_t crossfade (uint8_t a, uint8_t b, uint8_t alpha) {
  return (a*alpha + b*(255-alpha)) >> 8;
}






void hue_to_rgb(uint8_t hue, uint8_t& r, uint8_t& g, uint8_t& b)
{
	if(hue <= 42)
	{
		// 0 -> 42 : red -> yellow
		r = 0xFF;
		g = (hue - 0) * 6;
		b = 0;
	}
	else if (hue <= 85)
	{
		// 43 -> 85 : yellow -> green
		r = (85 - hue) * 6;
		g = 255;
		b = 0;
	}
	else if (hue <= 128)
	{
		// 86 -> 128 : green ->cyan
		r = 0;
		g = 255;
		b = (hue - 86) * 6;
	}
	else if (hue <= 171)
	{
		// 129 -> 171 : cyan -> blue
		r = 0;
		g = (171 - hue) * 6;
		b = 255;
	}
	else if (hue <= 214)
	{
		// 172 -> 214 : blue -> magenta
		r = (hue - 172) * 6;
		g = 0;
		b = 255;
	}
	else 
	{
		// 215 -> 257 : magenta -> red
		r = 255;
		g = 0;
		b = (257 - hue) * 6;
	}
}

__attribute__((naked)) uint16_t fixmul2 ( uint16_t a, uint16_t b)
{
	// a = r25:r24
	// b = r23:r22
	// return = r25:r24
	// temp: r27:r26
	
	asm("clr r26");
	asm("clr r27");
	
	// ah * bl
	asm("mul r25,r22");
	asm("movw r26, r0");
	
	// al * bh
	asm("mul r24, r23");
	asm("add r26, r0");
	asm("adc r27, r1");
	
	// ah * bh
	asm("mul r25, r23");
	asm("add r27, r0");
	
	// al * bl & clear r1
	asm("mul r24, r22");
	asm("add r26, r1");
	asm("clr r1");
	asm("adc r27, r1");

	// done
	asm("movw r24, r26");
	asm("ret");
}

__attribute__((noinline)) uint16_t fixmul ( uint16_t a, uint16_t b)
{
	uint8_t ah = a >> 8;
	uint8_t al = a;
	uint8_t bh = b >> 8;
	uint8_t bl = b;
	
	uint16_t out1 = (al * bl) >> 8;
	uint16_t out2 = (al * bh);
	uint16_t out3 = (ah * bl);
	uint16_t out4 = (ah * bh) << 8;
	
	return out1 + out2 + out3 + out4;
}	
