#include "Config.h"

void red_test() {
	static uint16_t timer;
	
	const int step = 35;
	const int speed = 3;
	
	timer += speed;
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		r[i] = pgm_read_byte(gammasin + t);
	}
}	

void green_test() {
	static uint16_t timer;
	
	const int step = 35;
	const int speed = 3;
	
	timer += speed;
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		g[i] = pgm_read_byte(gammasin + t);
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
		b[i] = pgm_read_byte(gammasin + t);
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

void pattern() {
	/*
	r[0] = g[0] = b[0] = bright1 >> 2;
	r[1] = g[1] = b[1] = bright1 >> 2;
	r[2] = g[2] = b[2] = bright2;
	r[3] = g[3] = b[3] = bright2;
	r[4] = g[4] = b[4] = bright2;
	r[5] = g[5] = b[5] = bright2;
	r[6] = g[6] = b[6] = bright1 >> 2;
	r[7] = g[7] = b[7] = bright1 >> 2;
	*/
	/*
	g[0] = (bright2 > 16 + 0) ? 0xFF : 0x00;
	g[1] = (bright2 > 16 + 32) ? 0xFF : 0x00;
	g[2] = (bright2 > 16 + 64) ? 0xFF : 0x00;
	g[3] = (bright2 > 16 + 96) ? 0xFF : 0x00;
	g[4] = (bright2 > 16 + 128) ? 0xFF : 0x00;
	g[5] = (bright2 > 16 + 160) ? 0xFF : 0x00;
	g[6] = (bright2 > 16 + 192) ? 0xFF : 0x00;
	g[7] = (bright2 > 16 + 224) ? 0xFF : 0x00;
	*/
}

void UpdateAudio ( int16_t sample );

uint16_t dummy_signal;
uint16_t dummy_trigger;
uint16_t dummy_bright1;
uint16_t dummy_bright2;

__attribute__((noinline)) void UpdateBassA() {
	uint16_t temp = dummy_bright1;
	if(dummy_signal >= dummy_trigger)
	{
		temp = (temp <= (65535 - BRIGHT2_UP)) ? temp + BRIGHT2_UP : 65535;
	}
	else
	{
		temp = (temp >= BRIGHT2_DOWN) ? temp - BRIGHT2_DOWN : 0;
	}
	dummy_bright1 = temp;
}

// 30 cycles
__attribute__((naked)) void UpdateBassB() {
	asm("push r28");
	asm("push r29");
	asm("push r30");
	asm("push r31");
	
	asm("lds r28, dummy_signal + 0");
	asm("lds r29, dummy_signal + 1");
	asm("lds r30, dummy_trigger + 0");
	asm("lds r31, dummy_trigger + 1");
	asm("cp r28, r30");
	asm("cpc r29, r31");
	asm("brcs signal_low");
	asm("nop");
	
	asm("signal_high:");
	asm("lds r28, dummy_bright2 + 0");
	asm("lds r29, dummy_bright2 + 1");
	asm("ldi r30, %0" : : "M"(lo8(65535 - BRIGHT2_UP)) );
	asm("ldi r31, %0" : : "M"(hi8(65535 - BRIGHT2_UP)) );
	asm("cp r28, r30");
	asm("cpc r29, r31");
	asm("brcc bright_max");
	asm("subi r28, %0" : : "M"(lo8(-BRIGHT2_UP)) );
	asm("sbci r29, %0" : : "M"(hi8(-BRIGHT2_UP)) );
	asm("nop");
	asm("rjmp store_bright");

	asm("signal_low:");
	asm("lds r28, dummy_bright2 + 0");
	asm("lds r29, dummy_bright2 + 1");
	asm("ldi r30, %0" : : "M"(lo8(BRIGHT2_DOWN)) );
	asm("ldi r31, %0" : : "M"(hi8(BRIGHT2_DOWN)) );
	asm("cp r28, r30");
	asm("cpc r29, r31");
	asm("brcs bright_min");
	asm("subi r28, %0" : : "M"(lo8(BRIGHT2_DOWN)) );
	asm("sbci r29, %0" : : "M"(hi8(BRIGHT2_DOWN)) );
	asm("nop");
	asm("rjmp store_bright");

	asm("bright_max:");
	asm("ser r28");
	asm("ser r29");
	asm("rjmp store_bright");
	
	asm("bright_min:");
	asm("clr r28");
	asm("clr r29");
	asm("rjmp store_bright");

	asm("store_bright:");
	asm("sts dummy_bright2 + 0, r28");
	asm("sts dummy_bright2 + 1, r29");
	
	asm("pop r31");
	asm("pop r30");
	asm("pop r29");
	asm("pop r28");
	asm("ret");
}

int main(void)
{
	/*
	dummy_signal = 1000;
	dummy_trigger = 10000;
	dummy_bright1 = 218;
	dummy_bright2 = 218;
	
	for(uint16_t i = 0; i < 65535; i++) {
		dummy_bright1 = i;
		dummy_bright2 = i;
		
		UpdateBassA();
		UpdateBassB();
		
		if(dummy_bright1 != dummy_bright2) {
			asm("nop");
		}			
	}
	*/
	
	/*
	dummy_signal1 = 500;
	dummy_trigger1 = 100;
	dummy_bright1 = 65535 - 2185 - 3;
	UpdateBassA();

	dummy_signal1 = 500;
	dummy_trigger1 = 100;
	dummy_bright1 = 65535 - 2185 + 3;
	UpdateBassA();

	dummy_signal1 = 100;
	dummy_trigger1 = 500;
	dummy_bright1 = 218 + 3;
	UpdateBassA();

	dummy_signal1 = 100;
	dummy_trigger1 = 500;
	dummy_bright1 = 218 - 3;
	UpdateBassA();
	*/

	// 38
	dummy_signal = 500;
	dummy_trigger = 100;
	dummy_bright2 = 65535 - BRIGHT2_UP - 3;
	UpdateBassB();

	// 40
	dummy_signal = 500;
	dummy_trigger = 100;
	dummy_bright2 = 65535 - BRIGHT2_UP + 3;
	UpdateBassB();

	// 39
	dummy_signal = 100;
	dummy_trigger = 500;
	dummy_bright2 = BRIGHT2_DOWN + 3;
	UpdateBassB();

	// 39
	dummy_signal = 100;
	dummy_trigger = 500;
	dummy_bright2 = BRIGHT2_DOWN - 3;
	UpdateBassB();

	
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