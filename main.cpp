#include "Defines.h"
#include "Patterns.h"

#include "AudioProcessing.h"
#include "LEDDriver.h"
#include "Tables.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

void testLEDs() {
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
// Set up ADC. Default config assumes F_CPU == 8000000
// We sample at 1000000 / (16*14) = ~4464 hz

#if F_CPU == 8000000
void SetupADC() {
	// conversion takes 28 uS
	ADMUX  = bit(MUX0) | bit(ADLAR);
	ADCSRA = bit(ADEN) | bit(ADPS2);
	sbi(ADCSRA,ADSC);
}	
#endif


void red_test() {
	static uint16_t timer;
	
	const int step = 35;
	const int speed = 3;
	
	timer += speed;
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		r[i] = getGammaSin(t);
	}
}	

void green_test() {
	static uint16_t timer;
	
	const int step = 35;
	const int speed = 30;
	
	timer += speed;
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		g[i] = getGammaSin(t);
	}
}

void blue_test() {
	static uint16_t timer;
	
	const int step = 35;
	const int speed = 3;
	
	timer += speed;
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		b[i] = getGammaSin(t);
	}
}

void audio_test() {
	r[0] = g[0] = bright1 >> 2;
	r[1] = g[1] = bright1 >> 2;
	r[2] = b[2] = bright2;
	r[3] = g[3] = b[3] = bright2;
	r[4] = g[4] = b[4] = bright2;
	r[5] = b[5] = bright2;
	r[6] = g[6] = bright1 >> 2;
	r[7] = g[7] = bright1 >> 2;
}	

void (*pattern_callback)();

int main(void)
{
	// Turn off the serial interface, which the bootloader leaves on by default.
	UCSR0B &= ~(1 << RXEN0);
	UCSR0B &= ~(1 << TXEN0);
	PRR = bit(PRTWI) | bit(PRTIM2) | bit(PRTIM0) | bit(PRSPI) | bit(PRUSART0);
	
	// Turn on status LEDs
	
	DDRC |= bit(2);
	DDRC |= bit(3);
	
	SetupADC();
	SetupLEDs();
	
	pattern_callback = audio_test;
	
	while(1) {
		while(!blank);
		swap();
		
		while(blank);
		pattern_callback();
		UpdateAudioSync();
		//PORTC ^= bit(3);
		//_delay_ms(1);
	}		
}