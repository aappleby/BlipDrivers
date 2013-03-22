#include "Patterns.h"
#include "LEDDriver.h"
#include "Bobs.h"
#include "Colors.h"
#include "Sleep.h"

#include <math.h>
#include <stdio.h>
#include <stdfix.h>

#define F_CPU 8000000

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

#define bit(A)   (1 << A)
#define sbi(p,b) { p |= (unsigned char)bit(b); }
#define cbi(p,b) { p &= (unsigned char)~bit(b); }

void testLEDs() {
	// Turn off the serial interface, which the bootloader leaves on by default.
	UCSR0B &= ~(1 << RXEN0);
	UCSR0B &= ~(1 << TXEN0);
	
	DDRB = 0x04;
	PORTB = 0x00;
	
	DDRD = 0xFF;
	PORTD = 0x01;
	
	while(1)
	{
		DDRB = 0x01; _delay_ms(200);
		DDRB = 0x02; _delay_ms(200);
		DDRB = 0x04; _delay_ms(200);
		PORTD = (PORTD << 1) | (PORTD >> 7);
	}	
}

//---------------------------------------------------------------------------

void dumb() {
	clear();
	if(blip_tick & (1 << 12)) pixels[(blip_tick >> 9) & 7].r = 0xFF;
	if(blip_tick & (1 << 13)) pixels[(blip_tick >> 9) & 7].g = 0xFF;
	if(blip_tick & (1 << 14)) pixels[(blip_tick >> 9) & 7].b = 0xFF;
}

void white() {
	for(int i = 0; i < 8; i++) {
		pixels[i].r =0xFF;
		pixels[i].g =0xFF;
		pixels[i].b =0xFF;
	}
}





void hsv()
{
	uint8_t h = blip_tick >> 9;
	
	for(int i = 0; i < 8; i++)
	{
		hue_to_rgb(h, pixels[i].r, pixels[i].g, pixels[i].b);
		h += 32;
	}
}

void hsv2()
{
  uint32_t h = blip_tick >> 3;
  
  for(int i = 0; i < 8; i++)
  {
    hue_to_rgb2(h, pixels[i].r, pixels[i].g, pixels[i].b);
    h += 180;
  }
}

extern const uint8_t gammasin[] PROGMEM;
extern const uint8_t pulse_5_6[256] PROGMEM;
extern const uint8_t pulse_2_6[256] PROGMEM;

void dueling_sines()
{
  uint32_t h = blip_tick >> 7;
  
  uint8_t sinA = pgm_read_byte(pulse_2_6 + (uint8_t)h);
  
  float t = float(blip_tick) / (256.0f * 128.0f);
  t = fmod(t, 1.0);
  t = pow(t, 1.0 / 2.0);
  t *= 3.141592653589793238;
  t = sin(t);
  t = pow(t, 6.0);
  uint8_t sinB = floor(t * 255.0 + 0.5);
  
  
  pixels[0].g = sinA;
  pixels[1].g = sinB;
}


typedef void (*pattern_callback)();

int pattern_index = 0;

pattern_callback patterns[] = {
	//Speed2,
	//Speed,
	//cie_test,
  dueling_sines,
  hsv2,
	hsv,
	PulseFade,
	Sparklefest,
	red_test,
	green_test,
	blue_test,
	audio_test,
	VUMeter,
	RGBWaves,
	StartupPattern,
	FastWaves,
	Sparkles,
	Scroller,
	SpaceZoom,
	crosscross,
};

void pll_test()
{
	pixels[0].r = bright2;
	
	//static uint32_t speed = 12000;
	//static uint32_t phase = 0;
	
	static uint8_t s0 = 0;
	static uint8_t s1 = 0;
	static uint8_t s2 = 0;
	
	uint8_t b = ibright2 >> 8;
	
	s0 = s1;
	s1 = s2;
	s2 = b;
	
	if(s2 < 32) return;
	if(s2 > (256-32)) return;
	
	if(s2 > s1)
	{
		// rising signal
		pixels[4].g = 255;		
	}
	
	if(s2 < s1)
	{
		pixels[5].r = 255;
	}
}

// s.3.12
__attribute__((noinline)) float floatsin(float a) {
  uint8_t x = a * 256;
  float result = pgm_read_float(gammasin + 4 * x);
  return result;
  //uint8_t b = pgm_read_byte(gammasin + x);
  //float result = b / 255.0;
  //return result;
}

__attribute__((noinline)) uint8_t lerp(uint8_t x1, uint8_t x2, uint8_t f1) {
  uint8_t f2 = ~f1;
  
  return (x2*f1 + x1*f2 + x1) >> 8;
  
  //return ((x1 * (255 - f) + x2 * f) + 128) >> 8;
}

__attribute__((noinline)) uint16_t lerp16(uint16_t x1, uint16_t x2, uint8_t t) {
  uint8_t s = ~t;
  return (uint32_t(x2)*t + uint32_t(x1)*s + x1) >> 8;
}

__attribute__((noinline)) uint8_t tablelerp8(const uint8_t* table, uint16_t x) {
  uint8_t x1 = x >> 8;
  uint8_t x2 = x1 + 1;
  uint8_t t = x;
  
  uint16_t y1 = pgm_read_byte(table + x1);
  uint16_t y2 = pgm_read_byte(table + x2);
  
  uint8_t s = ~t;
  return (y2*t + y1*s + y1) >> 8;
}

/*__attribute__((aligned(256)))*/ const uint8_t doesthiswork[256] PROGMEM = {1, 2, 4, 5,6};


__attribute__((naked)) uint8_t tablelerp8_asm(const uint8_t* table, uint16_t x) {
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

// Interpolate between two 8-bit numbers, 21 cycles.
// 24 = x1, r22 = x2, r20 = t
// return in r24
__attribute__((naked)) uint8_t lerp8_asm(uint8_t x1, uint8_t x2, uint8_t t) {
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

// Interpolate between two 16-bit numbers, 34 cycles.
// r25:r24 = x1, r23:r22 = x2, r20 = t
// return in r25:r24
__attribute__((naked)) uint16_t lerp16_asm(uint16_t x1, uint16_t x2, uint8_t t) {
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


__attribute__((noinline)) uint8_t lerplookup(uint16_t x) {
  uint8_t x1 = (x >> 8);
  uint8_t x2 = x1 + 1;
  
  uint8_t y1 = pgm_read_byte(gammasin + x1);
  uint8_t y2 = pgm_read_byte(gammasin + x2);
  
  return lerp(y1, y2, x);
}  

volatile float a = 1.14159289765181352;
volatile float b = 2.8798456156;
volatile float c = 14;
volatile uint8_t d = 1;
volatile uint16_t e = 1;

extern const uint8_t duh[] PROGMEM = { 17, 212 };

int main(void)
{
  e = tablelerp8(duh, 128);
  
  for(int i = 0; i < 30000; i++) {
    uint32_t x1 = xor128() & 0xFF;
    uint32_t x2 = xor128() & 0xFF;
    uint8_t t = xor128();
    
    uint16_t y1 = lerp16(x1, x2, t);
    uint16_t y2 = lerp8_asm(x1, x2, t);
    
    if(y1 != y2) {
      e = 10;
    }      
  }    
 
  e = 12;
  return 0;
  
	SetupLEDs();
	
	while(1) {
		//clear();
		//Speed();
		//button_test();
		//blip_swap();
		
    UpdateSleep();
    
    if((buttonstate1 == 1) && (debounce_down1 > 256)) {
      //pattern_callback = red_test;
      pattern_index = (pattern_index + 1) % 14;
      debounce_down1 = 0;
    }
		
		clear();
		patterns[pattern_index]();
		blip_swap();
	}		
}


