#include "Config.h"

void pattern() {
	/*
	static uint16_t timer;
	
	const int step = 35;
	const int speed = 3;
	
	timer += speed;
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		b[i] = pgm_read_byte(gammasin + t);
	}
	*/
	r[0] = g[0] = b[0] = bright1 >> 2;
	r[1] = g[1] = b[1] = bright1 >> 2;
	r[2] = g[2] = b[2] = bright2;
	r[3] = g[3] = b[3] = bright2;
	r[4] = g[4] = b[4] = bright2;
	r[5] = g[5] = b[5] = bright2;
	r[6] = g[6] = b[6] = bright1 >> 2;
	r[7] = g[7] = b[7] = bright1 >> 2;
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

volatile uint8_t button_up;
volatile uint8_t button_down;

void UpdateAudio ( int16_t sample );

int16_t blep;

/*
__attribute__((naked)) void myabs() {
	asm("lds r24, blep + 0");
	asm("lds r25, blep + 1");
	00000529  SBRS R25,7		Skip if bit in register set 
	0000052A  RET 		Subroutine return 
	0000052B  COM R25		One's complement 
	0000052C  NEG R24		Two's complement 
	0000052D  SBCI R25,0xFF		Subtract immediate with carry 
	0000052E  STS 0x0120,R25		Store direct to data space 
	00000530  STS 0x011F,R24		Store direct to data space 
	asm("sts blep + 0, r24");
	asm("sts blep + 1, r25");
	asm("ret");
}
*/

__attribute__((noinline)) void myabs() {
	blep = (blep < 0) ? -blep : blep+123;
}

__attribute__((naked)) void myabs2() {
	asm("lds r24, blep + 0");
	asm("lds r25, blep + 1");

	asm("sbrc r25, 7");
	asm("
	
	00000529  SBRC R25,7		Skip if bit in register cleared 
	0000052A  RJMP PC+0x0008		Relative jump 
	0000052B  SUBI R24,0x85		Subtract immediate 
	0000052C  SBCI R25,0xFF		Subtract immediate with carry 
	0000052D  STS 0x0120,R25		Store direct to data space 
	0000052F  STS 0x011F,R24		Store direct to data space 
	asm("sts blep + 0, r24");
	asm("sts blep + 1, r25");
	asm("rjmp abs_done");
	
	00000532  COM R25		One's complement 
	00000533  NEG R24		Two's complement 
	00000534  SBCI R25,0xFF		Subtract immediate with carry 
	00000535  STS 0x0120,R25		Store direct to data space 
	00000537  STS 0x011F,R24		Store direct to data space 
	
abs_done:
	asm("ret");
}




int main(void)
{
	myabs();
	
	// Turn off the serial interface, which the bootloader leaves on by default.
	UCSR0B &= ~(1 << RXEN0);
	UCSR0B &= ~(1 << TXEN0);
	PRR = bit(PRTWI) | bit(PRTIM2) | bit(PRTIM0) | bit(PRSPI) | bit(PRUSART0);
	
	// Turn on status LEDs
	
	DDRC |= bit(2);
	DDRC |= bit(3);
	
	SetupADC();
	SetupLEDs();
	
	pattern_callback = pattern;
	
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