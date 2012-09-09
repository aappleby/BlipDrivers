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

uint16_t dummy_trigger1;
uint16_t dummy_accum1;
uint8_t dummy_tickcount1;

// 28 cycles
/*
__attribute__((naked)) void Adapt1A() {
	asm("lds r30, dummy_tickcount1");
	asm("tst r30");
	asm("breq trig1_adapt");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("ret");
	
	asm("trig1_adapt:");
	asm("lds r30, dummy_trigger1 + 0");
	asm("lds r31, dummy_trigger1 + 1");
	asm("mov r20, r31");
	asm("lsr r20");
	asm("lsr r20");
	
	// Increase trigger if accum >= 4096, otherwise decrease.
	asm("lds r21, dummy_accum1 + 1");
	asm("andi r21, 0xF0");
	asm("breq trig1_down");
	
	asm("trig1_up:");
	asm("clr r21");
	asm("adiw r30, 1");
	asm("add r30, r20");
	asm("adc r31, r21");
	asm("jmp trig1_done");
	
	asm("trig1_down:");
	asm("clr r21");
	asm("sub r30, r20");
	asm("sbc r31, r21");
	asm("sbiw r30, 1");
	asm("nop");
	asm("nop");
	
	asm("trig1_done:");
	
	asm("sts dummy_trigger1 + 0, r30");
	asm("sts dummy_trigger1 + 1, r31");
	asm("ret");
}
*/

/*
uint16_t dummy_trigger2;
uint16_t dummy_accum2;

void Adapt1B() {
	uint16_t up = dummy_trigger2 >> 10;
	
	if(dummy_accum2 >> 12)
	{
		dummy_trigger2 += 1;
		dummy_trigger2 += up;
	}
	else
	{
		dummy_trigger2 -= up;
		dummy_trigger2 -= 1;
	}
}
*/	

void Clamp1();

int main(void)
{
	/*
	dummy_trigger1 = 100;
	dummy_accum1 = 0xFFFF;
	dummy_tickcount1 = 0;
	Adapt1A();
	
	dummy_trigger1 = 100;
	dummy_accum1 = 0xFFFF;
	dummy_tickcount1 = 1;
	Adapt1A();

	dummy_trigger1 = 100;
	dummy_accum1 = 0;
	dummy_tickcount1 = 1;
	Adapt1A();
	*/

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