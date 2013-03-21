#include "Patterns.h"
#include "LEDDriver.h"
#include "Bobs.h"
#include "Colors.h"
#include "Sleep.h"

#include <math.h>

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
	if(led_tick & (1 << 12)) pixels[(led_tick >> 9) & 7].r = 0xFF;
	if(led_tick & (1 << 13)) pixels[(led_tick >> 9) & 7].g = 0xFF;
	if(led_tick & (1 << 14)) pixels[(led_tick >> 9) & 7].b = 0xFF;
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
	uint8_t h = led_tick >> 9;
	
	for(int i = 0; i < 8; i++)
	{
		hue_to_rgb(h, pixels[i].r, pixels[i].g, pixels[i].b);
		h += 32;
	}
}

void hsv2()
{
  uint32_t h = led_tick >> 3;
  
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
  uint32_t h = led_tick >> 7;
  
  uint8_t sinA = pgm_read_byte(pulse_2_6 + (uint8_t)h);
  
  float t = float(led_tick) / (256.0f * 128.0f);
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

int main(void)
{
	SetupLEDs();
	
	while(1) {
		//clear();
		//Speed();
		//button_test();
		//swap();
		
    UpdateSleep();
    
    if((buttonstate1 == 1) && (debounce_down1 > 256)) {
      //pattern_callback = red_test;
      pattern_index = (pattern_index + 1) % 14;
      debounce_down1 = 0;
    }
		
		clear();
		patterns[pattern_index]();
		swap();
	}		
}


