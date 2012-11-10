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
	PulseFade,
	Sparklefest,
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

uint16_t blarp1 = 0x0001;
uint16_t blarp2 = 0x0002;

__attribute__((naked)) void saturation_test() {
	asm("lds r20, blarp1 + 0");
	asm("lds r21, blarp1 + 1");
	asm("lds r22, blarp2 + 0");
	asm("lds r23, blarp2 + 1");
	asm("sub r20, r22");
	asm("sbc r21, r23");
	asm("in r24, 0x3F");
	asm("sbrc r24,0");
	asm("ldi r20, 0xFF");
	asm("sbrc r24, 0");
	asm("ldi r21, 0xFF");
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


int main(void)
{
	SetupLEDs();
	
	while(1) {
		//clear();
		//button_test();
		//swap();
		//continue;
		
		
		if((buttonstate1 == 0) && (debounce_down1 > 16384)) {
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
			while(buttonstate1 == 0);
			GoToSleep();
			
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
			while(buttonstate1 == 0);
			debounce_down1 = 0;
			debounce_up1 = 0;
		}
		
		if((buttonstate1 == 1) && (debounce_down1 > 256)) {
			//pattern_callback = red_test;
			pattern_index = (pattern_index + 1) % 13;
			debounce_down1 = 0;
		}
		
		clear();
		patterns[pattern_index]();
		swap();
	}		
}


