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


void test_wdt() {
	uint8_t sleep_pattern = 0x01;
	
	sei();
	
	UCSR0A = 0;
	UCSR0B = 0;
	
	bool awake = true;
	
	int16_t debounce = 0;

	DDRC = 0x00;
	PORTC = 0x02;

	DDRD = 0xFF;
	PORTD = 0x01;
				
	DDRB = 0x02;
	PORTB = 0x00;
	
	sleep_bod_disable();

	while(1) {
		if(awake) {
			PORTD ^= 0x01;
			
			if(PINC & 0x02) {
				debounce = 0;
			} else {
				debounce++;
			}
			
			if(debounce == 3) {
				// turn off LED
				PORTD = 0x00;

				// reset button
				debounce = 0;
				
				// wait for button to go high again
				while((PINC & 0x02) == 0);
				
				// time to go to sleep

				ADMUX  = 0;
				ADCSRA = 0;
				
				PRR = bit(PRTWI) | bit(PRTIM2) | bit(PRTIM0) | bit(PRTIM1) | bit(PRSPI) | bit(PRUSART0) | bit(PRADC);
				SMCR = bit(SE) | bit(SM1);
				WDTCSR = bit(WDCE) | bit(WDE);
				WDTCSR = bit(WDIE) | bit(WDP1) | bit(WDP0);
				
				awake = false;
				continue;
			}

			_delay_ms(100);
			
		}
		else
		{
			asm("sleep");
			
			if(PINC & 0x02) {
				debounce = 0;
			} else {
				debounce++;
			}
			
			if(debounce > 1) {
				// wake up
				DDRD = 0xFF;
				PORTD = 0x01;
			
				DDRB = 0x02;
				PORTB = 0x00;
				
				// wait for button to go high again
				while((PINC & 0x02) == 0);
				
				debounce = 0;
				
				WDTCSR = bit(WDCE) | bit(WDE);
				WDTCSR = 0;

				SMCR = 0;

				PRR = bit(PRTWI) | bit(PRTIM2) | bit(PRTIM0) | bit(PRSPI) | bit(PRUSART0);

				ADMUX  = 0 | bit(ADLAR);
				ADCSRA = bit(ADEN) | bit(ADPS2) | bit(ADPS0);

				awake = true;
				continue;
			}
			
			DDRD = 0xFF;
			PORTD = sleep_pattern;
			sleep_pattern = (sleep_pattern << 7) | (sleep_pattern >> 1);
		
			DDRB = 0x01;
			PORTB = 0x00;

			_delay_us(10);
		
			DDRD = 0x00;
			PORTD = 0x00;
		
			DDRB = 0x00;
			PORTB = 0x00;
		}
	}
}

void scaler_test() {
	uint8_t pattern = 0x01;
	while(1) {
		DDRD = 0xFF;
		PORTD = pattern;
		pattern = (pattern << 7) | (pattern >> 1);
			
		DDRB = 0x01;
		PORTB = 0x00;

		_delay_us(10);
			
		DDRD = 0x00;
		PORTD = 0x00;
			
		DDRB = 0x00;
		PORTB = 0x00;
	}		
}


int main(void)
{
	//test_wdt();
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