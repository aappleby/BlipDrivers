#include "Patterns.h"

#include "LEDDriver.h"

#define F_CPU 8000000

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
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

void (*pattern_callback)();

void dumb() {
	clear();
	if(led_tick & (1 << 12)) pixels[(led_tick >> 9) & 7].r = 0xFF;
	if(led_tick & (1 << 13)) pixels[(led_tick >> 9) & 7].g = 0xFF;
	if(led_tick & (1 << 14)) pixels[(led_tick >> 9) & 7].b = 0xFF;
}

// watchdog interrupt has to exist or the chip resets.
ISR(WDT_vect) {}

uint16_t debounce_up = 0;
uint16_t debounce_down = 0;

__attribute__((naked)) void update_button() {
	// if(PINC & BUTTON) { if(debounce_up < 0x8000) debounce_up++; debounce_down = 0; }
    // else              { if(debounce_down < 0x8000) debounce_down++; debounce_up = 0; }
	// 22 cycles
	asm("sbis %0, %1" : : "I"(_SFR_IO_ADDR(PINC)), "X"(1));
	asm("jmp button_down");

	asm("button_up:");
	asm("lds r24, debounce_up + 0");
	asm("lds r25, debounce_up + 1");

	asm("sbrs r25, 7");
	asm("subi r24, 0xFF");
	asm("sbrs r25, 7");
	asm("sbci r25, 0xFF");
	
	asm("sts debounce_up + 0, r24");
	asm("sts debounce_up + 1, r25");
	asm("sts debounce_down + 0, r1");
	asm("sts debounce_down + 1, r1");

	asm("jmp button_done");

	asm("button_down:");
	asm("lds r24, debounce_down + 0");
	asm("lds r25, debounce_down + 1");

	asm("sbrs r25, 7");
	asm("subi r24, 0xFF");
	asm("sbrs r25, 7");
	asm("sbci r25, 0xFF");

	asm("sts debounce_down + 0, r24");
	asm("sts debounce_down + 1, r25");
	asm("sts debounce_up + 0, r1");
	asm("sts debounce_up + 1, r1");
	asm("nop");
	asm("nop");

	asm("button_done:");
	asm("ret");
}



__attribute__((noinline)) void update_button2() {
	if(PINC & 0x02) {
		debounce_up++;
		if(debounce_up & 0x8000) debounce_up -= 0x0100;
		debounce_down = 0;
	} else {
		debounce_up = 0;
		debounce_down++;
		if(debounce_down & 0x8000) debounce_down -= 0x0100;
	}
}

void wake_up() {
	// wake up
	DDRD = 0xFF;
	PORTD = 0x01;
				
	DDRB = 0x02;
	PORTB = 0x00;
				
	// wait for button to go high again
	while(debounce_up < 100) { update_button(); }
				
	WDTCSR = bit(WDCE) | bit(WDE);
	WDTCSR = 0;

	SMCR = 0;

	PRR = bit(PRTWI) | bit(PRTIM2) | bit(PRTIM0) | bit(PRSPI) | bit(PRUSART0);

	ADMUX  = 0 | bit(ADLAR);
	ADCSRA = bit(ADEN) | bit(ADPS2) | bit(ADPS0);
}

const uint8_t gammasin2[] PROGMEM =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,2,2,2,2,3,3,4,4,5,5,6,7,8,9,9,10,
	11,13,14,15,16,18,19,21,23,24,26,28,30,32,34,37,39,41,44,46,49,52,55,58,61,
	64,67,70,73,77,80,84,87,91,95,98,102,106,110,114,118,122,126,130,134,138,142,146,
	150,154,158,162,166,170,174,178,182,186,190,193,197,200,204,207,211,214,217,220,
	223,226,228,231,234,236,238,240,242,244,246,247,249,250,251,252,253,254,254,255,
	255,255,255,255,254,254,253,252,251,250,249,247,246,244,242,240,238,236,234,231,
	228,226,223,220,217,214,211,207,204,200,197,193,190,186,182,178,174,170,166,162,
	158,154,150,146,142,138,134,130,126,122,118,114,110,106,102,98,95,91,87,84,80,77,
	73,70,67,64,61,58,55,52,49,46,44,41,39,37,34,32,30,28,26,24,23,21,19,18,16,15,14,
	13,11,10,9,9,8,7,6,5,5,4,4,3,3,2,2,2,2,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

void do_sleep() {
	DDRD = 0xFF;
	PORTD = 0x00;
	DDRB = 0x01;
	PORTB = 0x00;
	
	uint8_t cursor = 0;
	uint8_t column = 1;
	while(1) {
		asm("sleep");
		
		update_button();
		
		if(debounce_down > 10) {
			wake_up();
			return;
		}

		if(cursor == 0) {
			column = (column << 1) | (column >> 7);
		}			
		uint8_t bright = pgm_read_byte(gammasin2 + cursor) >> 2;
		if(bright) {
			PORTD = column; 
			for(uint8_t i = 0; i < bright; i++) { asm("nop"); }
			PORTD = 0;
		}			
		cursor += 1;
	}
	DDRD = 0x00; PORTD = 0x00; DDRB = 0x00; PORTB = 0x00;
}

extern "C" {
	void ShutStuffDown();
};

void go_to_sleep() {
	
	DDRC = 0x00;
	PORTC = 0x02;
	
	// turn off LEDs
	PORTD = 0x00;

	// wait for button to go high again
	while(debounce_up < 100) { update_button(); }
			
	// time to go to sleep
	
	ShutStuffDown();
	sleep_bod_disable();

	SMCR = bit(SE) | bit(SM1);
	WDTCSR = bit(WDCE) | bit(WDE);
	WDTCSR = bit(WDIE);// | bit(WDP0);// | bit(WDP1);
			
	do_sleep();
}

void test_wdt() {
	sei();
	
	ShutStuffDown();
	
	DDRC = 0x00;
	PORTC = 0x02;

	DDRD = 0xFF;
	PORTD = 0x01;
				
	DDRB = 0x02;
	PORTB = 0x00;
	
	while(1) {
		PORTD ^= 0x01;
			
		update_button();

		if(debounce_down > 3) {
			go_to_sleep();			
		}

		_delay_ms(100);
	}
}

void white() {
	for(int i = 0; i < 8; i++) {
		pixels[i].r =0xFF;
		pixels[i].g =0xFF;
		pixels[i].b =0xFF;
	}
}

int main(void)
{
	debounce_up = 0x8000;
	debounce_down = 0x8000;
	
	update_button();
	update_button2();
	
	test_wdt();
	SetupLEDs();
	
	pattern_callback = RGBWaves;
	while(1) {
		pattern_callback();
		//sbi(PORTC,3);
		//_delay_ms(1);
		swap();
		//cbi(PORTC,3);
		//_delay_ms(1);
	}		
}