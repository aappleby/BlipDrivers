#include "Defines.h"
#include "Patterns.h"

#include "LEDDriver.h"
#include "Tables.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

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

int main(void)
{
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