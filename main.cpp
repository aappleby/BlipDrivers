#include "Patterns.h"

#include "LEDDriver.h"

#define F_CPU 8000000

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

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

extern "C" {
	void GoToSleep();
};


void white() {
	for(int i = 0; i < 8; i++) {
		pixels[i].r =0xFF;
		pixels[i].g =0xFF;
		pixels[i].b =0xFF;
	}
}

Pixel backup[8];

void fade (uint8_t& x) {
	uint16_t y = (x * 251);
	x = y >> 8;
}

void hsv()
{
	uint8_t h = led_tick >> 8;
	
	for(int i = 0; i < 8; i++) {
		
		if(h <= 42) {
			// 0 -> 42 : red -> yellow
			pixels[i].r = 0xFF;
			pixels[i].g = (h - 0) * 6;
			pixels[i].b = 0;
		} else if (h <= 85) {
			// 43 -> 85 : yellow -> green
			pixels[i].r = (85 - h) * 6;
			pixels[i].g = 255;
			pixels[i].b = 0;
		} else if (h <= 128) {
			// 86 -> 128 : green ->cyan
			pixels[i].r = 0;
			pixels[i].g = 255;
			pixels[i].b = (h - 86) * 6;
		} else if (h <= 171) {
			// 129 -> 171 : cyan -> blue
			pixels[i].r = 0;
			pixels[i].g = (171 - h) * 6;
			pixels[i].b = 255;
		} else if (h <= 214) {
			// 172 -> 214 : blue -> magenta
			pixels[i].r = (h - 172) * 6;
			pixels[i].g = 0;
			pixels[i].b = 255;
		} else {
			// 215 -> 257 : magenta -> red
			pixels[i].r = 255;
			pixels[i].g = 0;
			pixels[i].b = (257 - h) * 6;
		}
		h += 32;
	}
}

typedef void (*pattern_callback)();

int pattern_index = 0;

pattern_callback patterns[] = {
	red_test,
	green_test,
	blue_test,
	audio_test,
	VUMeter,
	Speed,
	RGBWaves,
	StartupPattern,
	FastWaves,
	Sparkles,
	Scroller,
	SpaceZoom,
	crosscross,
};



int main(void)
{
	SetupLEDs();
	
	while(1) {
		if((buttonstate == 0) && (debounce_down > 16384)) {
			uint32_t old_tick = led_tick;
			for(int i = 0; i < 8; i++) { backup[i] = pixels[i]; }
			for(uint16_t i = 0; i < 256; i++) {
				uint8_t f = ((255-i) * (255-i)) >> 8;
				for(int j = 0; j < 8; j++) {
					pixels[j].r = (backup[j].r * f) >> 8;
					pixels[j].g = (backup[j].g * f) >> 8;
					pixels[j].b = (backup[j].b * f) >> 8;
				}
				for(int j = 0; j < 13; j++) swap();
			}
			// wait for button release before going to sleep
			while(buttonstate == 0);
			GoToSleep();
			/*
			while(1) {
				clear();
				red_test();
				swap();
				if((buttonstate == 0) && (debounce_down > 1024)) {
					while(buttonstate == 0);
					break;
				}
			}
			*/
			
			for(uint16_t i = 0; i < 256; i++) {
				uint8_t f = (i * i) >> 8;
				for(int j = 0; j < 8; j++) {
					pixels[j].r = (backup[j].r * f) >> 8;
					pixels[j].g = (backup[j].g * f) >> 8;
					pixels[j].b = (backup[j].b * f) >> 8;
				}
				for(int j = 0; j < 13; j++) swap();
			}
			led_tick = old_tick;
			while(buttonstate == 0);
			debounce_down = 0;
			debounce_up = 0;
		}
		
		if((buttonstate == 1) && (debounce_down > 256)) {
			//pattern_callback = red_test;
			pattern_index = (pattern_index + 1) % 13;
			debounce_down = 0;
		}
		
		clear();
		patterns[pattern_index]();
		//sbi(PORTC,3);
		//_delay_ms(1);
		swap();
		//cbi(PORTC,3);
		//_delay_ms(1);
	}		
}


