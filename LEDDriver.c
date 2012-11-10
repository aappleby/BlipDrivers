#include "LEDDriver.h"

#ifndef F_CPU
#define F_CPU 8000000
#endif

#include <avr/interrupt.h>
#include <avr/pgmspace.h> 
#include <avr/sleep.h>

#define BOARDTYPE_POVLACE1

// fuses for ATMega328p -
// low:      0xE2
// high:     0xDA
// extended: 0xFF

//-----------------------------------------------------------------------------
// board configs

/*
// prototype 2 settings
#define PORT_SOURCE PORTD
#define PORT_SINK DDRB
#define SINK_GREEN 0x0C
#define SINK_RED 0x12
#define SINK_BLUE 0x01;
*/

//-----------------------------------------------------------------------------
// prototype 3 settings
/*
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

#define SOURCE_1 0x02
#define SOURCE_2 0x04
#define SOURCE_3 0x01
#define SOURCE_4 0x08
#define SOURCE_5 0x40
#define SOURCE_6 0x10
#define SOURCE_7 0x20
#define SOURCE_8 0x80

#define ADC_CHANNEL 1

#define SINK_RED   0x7D  // (~((1 << 1) | (1 << 7)))
#define SINK_GREEN 0xB3  // (~((1 << 2) | (1 << 3) | (1 << 6)))
#define SINK_BLUE  0xFE  // (~((1 << 0)))
*/

//-----------------------------------------------------------------------------
// test board from OSH Park

#ifdef BOARDTYPE_PROTO1
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

#define ADC_CHANNEL 0

#define SOURCE_1 0x02
#define SOURCE_2 0x04
#define SOURCE_3 0x01
#define SOURCE_4 0x08
#define SOURCE_5 0x10
#define SOURCE_6 0x40
#define SOURCE_7 0x20
#define SOURCE_8 0x80

#define SINK_RED   0xF1  // (~((1 << 1) | (1 << 7)))
#define SINK_GREEN 0x6E  // (~((1 << 2) | (1 << 3) | (1 << 6)))
#define SINK_BLUE  0x9F  // (~((1 << 0)))

#define BUTTON1_PIN 1

#define MIC_POWER 7
#define MIC_PIN 0
#endif

//-----------------------------------------------------------------------------
// test board from OSH Park 2

#ifdef BOARDTYPE_PROTO2
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

#define ADC_CHANNEL 0

#define SOURCE_1 0x02
#define SOURCE_2 0x04
#define SOURCE_3 0x01
#define SOURCE_4 0x08
#define SOURCE_5 0x10
#define SOURCE_6 0x40
#define SOURCE_7 0x20
#define SOURCE_8 0x80

#define SINK_RED 0x6E
#define SINK_GREEN 0xF1
#define SINK_BLUE 0x9F

#define BUTTON1_PIN 1
#define BUTTON2_PIN 1

#define MIC_POWER 2
#define MIC_PIN 7
#endif

//-----------------------------------------------------------------------------
// test board from OSH Park 3

#ifdef BOARDTYPE_PROTO3
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

#define ADC_CHANNEL 0

#define SOURCE_1 0x02
#define SOURCE_2 0x04
#define SOURCE_3 0x01
#define SOURCE_4 0x08
#define SOURCE_5 0x10
#define SOURCE_6 0x40
#define SOURCE_7 0x20
#define SOURCE_8 0x80

#define SINK_RED 0x6E
#define SINK_GREEN 0xF1
#define SINK_BLUE 0x9F

#define BUTTON1_PIN 3
#define BUTTON2_PIN 4

#define MIC_POWER 2
#define MIC_PIN 7
#endif

//-----------------------------------------------------------------------------
// POVLace 1

#ifdef BOARDTYPE_POVLACE1
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

#define SOURCE_1 0x02
#define SOURCE_2 0x04
#define SOURCE_3 0x01
#define SOURCE_4 0x08
#define SOURCE_5 0x10
#define SOURCE_6 0x40
#define SOURCE_7 0x20
#define SOURCE_8 0x80

#define SINK_RED   0xDE
#define SINK_GREEN 0xED
#define SINK_BLUE  0xF3

#define MIC_PIN 7
#define MIC_POWER 2
#define BUTTON1_PIN 3
#define BUTTON2_PIN 3
#endif

//-----------------------------------------------------------------------------

#define bit(A)   (1 << A)
#define sbi(p,b) { p |= (unsigned char)bit(b); }
#define cbi(p,b) { p &= (unsigned char)~bit(b); }
#define tbi(p,b) { p ^= (unsigned char)bit(b); }
#define gbi(p,b) (p & (unsigned char)bit(b))

#define lo8(A) (((uint16_t)A) & 0xFF)
#define hi8(A) (((uint16_t)A) >> 8)

//-----------------------------------------------------------------------------
// audio processing configs

#define TRIG1_CLAMP  60
#define BRIGHT1_UP   (65535 / 30)
#define BRIGHT1_DOWN (65535 / 300)

#define TRIG2_CLAMP  60
#define BRIGHT2_UP   (65535 / 40)
#define BRIGHT2_DOWN (65535 / 700)

//-----------------------------------------------------------------------------

const uint8_t sources[] PROGMEM =
{
	SOURCE_1, SOURCE_2, SOURCE_3, SOURCE_4,	SOURCE_5, SOURCE_6, SOURCE_7,
	SOURCE_8, SOURCE_7, SOURCE_6, SOURCE_5, SOURCE_4, SOURCE_3, SOURCE_2,
};

// Front buffer
uint8_t bits_RF[8];
uint8_t bits_GF[8];
uint8_t bits_BF[8];

uint8_t bits_RF2[8];
uint8_t bits_GF2[8];
uint8_t bits_BF2[8];

uint8_t bits_RF3[8];
uint8_t bits_GF3[8];
uint8_t bits_BF3[8];

/*
uint8_t r[8];
uint8_t g[8];
uint8_t b[8];
*/

struct Pixel pixels[8];

// Blanking interval
volatile uint8_t blank;

// PWM cycle tick, 4.096 kilohertz.
uint32_t led_tick;

// PWM callback function pointer dispatched by the timer interrupt.
void (*timer_callback)() ;

//-----------------------------------------------------------------------------
// Treble channel trigger value
uint16_t tmax1;

// Bass channel trigger value
uint16_t tmax2;

// Current sample, contains treble signal after bass/treble split.
uint16_t sample;

// Current bass sample.
uint16_t bass;

// DC filter accumulator
int16_t accumD;

// Bass filter accumulator
int16_t accumB;

// Brightness cursor, treble channel.
uint16_t ibright1;

// Brightness cursor, bass channel.
uint16_t ibright2;

// Brightness accumulator, treble channel/
uint16_t brightaccum1 = 0;

// Brightness accumulator, bass channel.
uint16_t brightaccum2 = 0;

// Output brightness, treble channel
uint8_t bright1 = 0;

// Output brightness, bass channel
uint8_t bright2 = 0;

// Internal tick count to trigger volume adaptation every 64 cycles.
uint8_t tickcount = 0;

// Button debounce counters
volatile uint8_t buttonstate1 = 0;
volatile uint16_t debounce_up1 = 0;
volatile uint16_t debounce_down1 = 0;

volatile uint8_t buttonstate2 = 0;
volatile uint16_t debounce_up2 = 0;
volatile uint16_t debounce_down2 = 0;

//-----------------------------------------------------------------------------

const uint8_t exptab[256] PROGMEM =
{
0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,5,5,
5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9,9,10,10,10,
10,10,11,11,11,11,12,12,12,12,13,13,13,14,14,14,14,15,15,15,16,16,16,17,
17,18,18,18,19,19,20,20,21,21,21,22,22,23,23,24,24,25,25,26,27,27,28,28,
29,30,30,31,32,32,33,34,35,35,36,37,38,39,39,40,41,42,43,44,45,46,47,48,
49,50,51,52,53,55,56,57,58,59,61,62,63,65,66,68,69,71,72,74,76,77,79,81,
82,84,86,88,90,92,94,96,98,100,102,105,107,109,112,114,117,119,122,124,
127,130,133,136,139,142,145,148,151,155,158,162,165,169,172,176,180,184,
188,192,196,201,205,210,214,219,224,229,234,239,244,250,255,
};

// numeric brightness -> perceived brightness

const uint8_t gammatab[256] PROGMEM = 
{
0,15,22,27,31,35,39,42,45,47,50,52,55,57,59,61,63,65,67,69,71,73,74,76,78,79,
81,82,84,85,87,88,90,91,93,94,95,97,98,99,100,102,103,104,105,107,108,109,110,
111,112,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,
131,132,133,134,135,136,137,138,139,140,141,141,142,143,144,145,146,147,148,
148,149,150,151,152,153,153,154,155,156,157,158,158,159,160,161,162,162,163,
164,165,165,166,167,168,168,169,170,171,171,172,173,174,174,175,176,177,177,
178,179,179,180,181,182,182,183,184,184,185,186,186,187,188,188,189,190,190,
191,192,192,193,194,194,195,196,196,197,198,198,199,200,200,201,201,202,203,
203,204,205,205,206,206,207,208,208,209,210,210,211,211,212,213,213,214,214,
215,216,216,217,217,218,218,219,220,220,221,221,222,222,223,224,224,225,225,
226,226,227,228,228,229,229,230,230,231,231,232,233,233,234,234,235,235,236,
236,237,237,238,238,239,240,240,241,241,242,242,243,243,244,244,245,245,246,
246,247,247,248,248,249,249,250,250,251,251,252,252,253,253,254,255,
};

//------------------------------------------------------------------------------
// New interrupt handlers

#define TIMEOUT_6R (65536 - 127)
#define TIMEOUT_6G (65536 - 127)
#define TIMEOUT_6B (65536 - 127)

#define TIMEOUT_7R (65536 - 300)
#define TIMEOUT_7G (65536 - 300)
#define TIMEOUT_7B (65536 - 300)

__attribute__((naked)) void bits_red_6() {
	// end previous pulse
	{
		asm("clr r30");
		asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	// Dead space between PWM cycles. Time spent here should cause
	// the overall PWM rate to be 4.096 khz, or ~1953 cycles.
	
	{
		// clear blanking flag (2 cycles)
		asm("sts blank, r30");

		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop");
	}		

	// switch to new sink
	asm("ldi r30, %0" : : "M" (SINK_RED));
	asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SINK)) );
	
	// send 3 cycle pulse
	{
		asm("lds r30, bits_RF + 0");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// send 6 cycle pulse
	{
		asm("lds r30, bits_RF + 1");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 3 cycle gap

	asm("nop"); asm("nop"); asm("nop");
	
	// send 11 cycle pulse
	{
		asm("lds r30, bits_RF + 2");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 8 cycle gap
	
	// save temp registers
	asm("push r25");
	asm("push r28");
	asm("push r29");
	asm("nop");
	asm("nop");
	
	// send 20 cycle pulse
	{
		asm("lds r25, bits_RF + 3");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 17 cycle gap
	
	// save extra temp registers
	asm("push r26");
	asm("push r27");
	
	// remove dc bias, part 1
	{
		// sample -= accumD;
		// accumD += (sample >> DCFILTER);

		asm("lds r28, %0" : : "X" (ADCL) );
		asm("lds r29, %0" : : "X" (ADCH) );
		asm("lds r30, accumD + 0");
		asm("lds r31, accumD + 1");
	
		asm("sub r28, r30");
		asm("sbc r29, r31");
	
		asm("movw r26, r28");
		asm("asr r27"); asm("ror r26");
	}		

	// send 40 cycle pulse
	{
		asm("lds r25, bits_RF + 4");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	
	// remove dc bias, part 2.
	{		
		asm("asr r27"); asm("ror r26");
		asm("asr r27"); asm("ror r26");
		asm("asr r27"); asm("ror r26");
		asm("asr r27"); asm("ror r26");
	
		asm("add r30, r26");
		asm("adc r31, r27");
	
		asm("sts accumD + 0, r30");
		asm("sts accumD + 1, r31");
	}		
	
	// split bass and treble
	{
		// sample -= accumB;
		// accumB += (sample >> BASSFILTER);
	
		asm("lds r30, accumB + 0");
		asm("lds r31, accumB + 1");
		asm("sub r28, r30");
		asm("sbc r29, r31");
	
		asm("movw r26, r28");
		asm("asr r27"); asm("ror r26");
		asm("asr r27"); asm("ror r26");
		asm("asr r27"); asm("ror r26");
	
		asm("add r30, r26");
		asm("adc r31, r27");
	
		asm("sts accumB + 0, r30");
		asm("sts accumB + 1, r31");
	}		

	asm("nop"); asm("nop"); asm("nop"); asm("nop"); 
	
	// send 80 cycle pulse
	{
		asm("lds r25, bits_RF + 5");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}

	// normalize bass
	{
		// bass = (accumB < 0) ? -accumB : accumB;
		asm("sbrc r31, 7");
		asm("jmp negate_bass");
		asm("nop");
		asm("jmp bass_done");
		asm("negate_bass:");
		asm("com r31");
		asm("neg r30");
		asm("sbci r31, 0xFF");
		asm("bass_done:");
	
		asm("sts bass + 0, r30");
		asm("sts bass + 1, r31");
	}		

	// normalize treble
	{	
		// if(sample < 0) sample = -sample;
		asm("sbrc 29, 7");
		asm("jmp negate_treb");
		asm("nop");
		asm("jmp treb_done");
		asm("negate_treb:");
		asm("com r29");
		asm("neg r28");
		asm("sbci r29, 0xFF");
		asm("treb_done:");
		
		// store sample.
		asm("sts sample + 0, r28");
		asm("sts sample + 1, r29");
	}		
	
	// increment audio tick, 5 cycles
	{
		// tickcount += 4;
		asm("lds r30, tickcount");
		asm("subi r30, 0xFC");
		asm("sts tickcount, r30");
	}		
	
	// adapt1
	{
		asm("lds r30, tickcount");
		asm("tst r30");
		asm("breq trig1_adapt");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop");
		asm("rjmp trig1_noadapt");
	
		asm("trig1_adapt:");
		asm("lds r30, tmax1 + 0");
		asm("lds r31, tmax1 + 1");
		asm("mov r28, r31");
		asm("lsr r28");
		asm("lsr r28");
	
		// Increase trigger if accum >= 4096, otherwise decrease.
		asm("lds r29, brightaccum1 + 1");
		asm("andi r29, 0xF0");
		asm("breq trig1_down");
	
		asm("trig1_up:");
		asm("clr r29");
		asm("adiw r30, 1");
		asm("add r30, r28");
		asm("adc r31, r29");
		asm("jmp trig1_done");
	
		asm("trig1_down:");
		asm("clr r29");
		asm("sub r30, r28");
		asm("sbc r31, r29");
		asm("sbiw r30, 1");
		asm("nop");
		asm("nop");
	
		asm("trig1_done:");
		asm("sts tmax1 + 0, r30");
		asm("sts tmax1 + 1, r31");
	
		asm("trig1_noadapt:");
	}		

	// restore extra temp registers
	asm("pop r27");
	asm("pop r26");

	// restore temp registers
	asm("pop r29");
	asm("pop r28");
	asm("pop r25");
	
	// set next callback, 6 cycles
	asm("ldi r30, pm_lo8(bits_red_7)");
	asm("ldi r31, pm_hi8(bits_red_7)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout, 6 cycles
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_6R)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_6R)) );
	asm("sts %0, r31" : : "X" (TCNT1H));
	asm("sts %0, r30" : : "X" (TCNT1L));

	// send 160 cycle pulse
	{
		asm("lds r30, bits_RF + 6");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	// Restore status register, R31, and R30.
	asm("pop r30");
	asm("out 0x3f, r30");
	asm("pop r31");
	asm("pop r30");

	// Done
	asm("reti");
}

//----------

__attribute__((naked)) void bits_red_7() {
	// set next callback
	asm("ldi r30, pm_lo8(bits_green_6)");
	asm("ldi r31, pm_hi8(bits_green_6)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");

	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_7R)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_7R)) );
	asm("sts %0, r31" : : "X" (TCNT1H));
	asm("sts %0, r30" : : "X" (TCNT1L));

	// send 320 cycle pulse
	{
		asm("lds r30, bits_RF + 7");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// Restore status register, R31, and R30.
	asm("pop r30");
	asm("out 0x3f, r30");
	asm("pop r31");
	asm("pop r30");

	// Done
	asm("reti");
}

//------------------------------------------------------------------------------

__attribute__((naked)) void bits_green_6() {
	// end previous pulse
	{
		asm("clr r30");
		asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	// switch to new sink
	asm("ldi r30, %0" : : "M" (SINK_GREEN));
	asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SINK)) );
	
	// send 3 cycle pulse
	{
		asm("lds r30, bits_GF + 0");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// send 6 cycle pulse
	{
		asm("lds r30, bits_GF + 1");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 3 cycle gap
	
	// clear the monitor bit (2 cycles)
	//asm("cbi %0, 2" : : "I"(_SFR_IO_ADDR(PORTC)) );
	asm("nop"); asm("nop"); asm("nop");
	
	// send 11 cycle pulse
	{
		asm("lds r30, bits_GF + 2");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 8 cycle gap
	
	// save temp registers
	
	asm("push r25");
	asm("push r28");
	asm("push r29");
	asm("nop");
	asm("nop");
	
	// send 20 cycle pulse
	{
		asm("lds r25, bits_GF + 3");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop");
	
	// send 40 cycle pulse
	{
		asm("lds r25, bits_GF + 4");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 37 cycle gap

	// clamp 2, 18 cycles
	{
		// if(tmax2 & 0x8000) tmax2 -= 256;
		// if(tmax2 < TRIG2_CLAMP) tmax2++;
		// 18 cycles
	
		asm("lds r30, tmax2 + 0");
		asm("lds r31, tmax2 + 1");

		// Clamp if above 32767
		asm("sbrc r31, 7");
		asm("subi r31, 0x01");
	
		// Clamp if below 60
		asm("clr r29");
		asm("cpi r30, 60");
		asm("cpc r31, r29");
		{
			asm("brge trig2_noclamp");
			asm("adiw r30, 1");
			asm("rjmp trig2_clampdone");
	
			asm("trig2_noclamp:");
			asm("nop");
			asm("nop");
			asm("nop");
			asm("trig2_clampdone:");
		}			
	
		asm("sts tmax2 + 0, r30");
		asm("sts tmax2 + 1, r31");
	}		

	// clamp 1, 18 cycles
	{
		// if(tmax1 & 0x8000) tmax1 -= 256;
		// if(tmax1 < TRIG1_CLAMP) tmax1++;
		// 18 cycles

		asm("lds r30, tmax1 + 0");
		asm("lds r31, tmax1 + 1");

		// Clamp if above 32767
		asm("sbrc r31, 7");
		asm("subi r31, 0x01");
	
		// Clamp if below 60
		asm("clr r29");
		asm("cpi r30, 60");
		asm("cpc r31, r29");
		{
			asm("brge trig1_noclamp");
			asm("adiw r30, 1");
			asm("rjmp trig1_clampdone");
	
			asm("trig1_noclamp:");
			asm("nop");
			asm("nop");
			asm("nop");
			asm("trig1_clampdone:");
		}			
	
		asm("sts tmax1 + 0, r30");
		asm("sts tmax1 + 1, r31");
	}		

	// 1 spare cycle
	asm("nop");

	// send 80 cycle pulse
	{
		asm("lds r25, bits_GF + 5");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	
	// adapt 2
	{
		asm("lds r30, tickcount");
		asm("tst r30");
		asm("breq trig2_adapt");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop");
		asm("rjmp trig2_noadapt");
	
		asm("trig2_adapt:");
		asm("lds r30, tmax2 + 0");
		asm("lds r31, tmax2 + 1");
		asm("mov r28, r31");
		asm("lsr r28");
		asm("lsr r28");
	
		// Increase trigger if accum >= 4096, otherwise decrease.
		asm("lds r29, brightaccum2 + 1");
		asm("andi r29, 0xF0");
		asm("breq trig2_down");
	
		asm("trig2_up:");
		asm("clr r29");
		asm("adiw r30, 1");
		asm("add r30, r28");
		asm("adc r31, r29");
		asm("jmp trig2_done");
	
		asm("trig2_down:");
		asm("clr r29");
		asm("sub r30, r28");
		asm("sbc r31, r29");
		asm("sbiw r30, 1");
		asm("nop");
		asm("nop");
	
		asm("trig2_done:");
		asm("sts tmax2 + 0, r30");
		asm("sts tmax2 + 1, r31");
	
		asm("trig2_noadapt:");
	}		

	// clear brightness accumulators when the tick rolls over.
	// 14 cycles
	{
		// if(tickcount == 0) { brightaccum1 = 0; brightaccum2 = 0; }
		asm("lds r30, tickcount");
		asm("tst r30");
		asm("brne noclear");
		asm("sts brightaccum1 + 0, r30");
		asm("sts brightaccum1 + 1, r30");
		asm("sts brightaccum2 + 0, r30");
		asm("sts brightaccum2 + 1, r30");
		asm("rjmp cleardone");
		asm("noclear:");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("cleardone:");
	}		

	// increment LED tick, 21 cycles
	{
		asm("clr r25");
		asm("lds r28, led_tick+0");
		asm("lds r29, led_tick+1");
		asm("lds r30, led_tick+2");
		asm("lds r31, led_tick+3");
		asm("adiw r28, 0x01");
		asm("adc r30, r25");
		asm("adc r31, r25");
		asm("sts led_tick+0, r28");
		asm("sts led_tick+1, r29");
		asm("sts led_tick+2, r30");
		asm("sts led_tick+3, r31");
	}
	asm("nop"); asm("nop");
	
	// set next callback
	asm("ldi r30, pm_lo8(bits_green_7)");
	asm("ldi r31, pm_hi8(bits_green_7)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_6G)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_6G)) );
	asm("sts %0, r31" : : "X" (TCNT1H));
	asm("sts %0, r30" : : "X" (TCNT1L));
	
	// send 160 cycle pulse
	{
		asm("lds r30, bits_GF + 6");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	// restore temp registers
	asm("pop r29");
	asm("pop r28");
	asm("pop r25");
	
	// Restore status register, R31, and R30.
	asm("pop r30");
	asm("out 0x3f, r30");
	asm("pop r31");
	asm("pop r30");

	// Done
	asm("reti");
}

__attribute__((naked)) void bits_green_7() {
	// set next callback
	asm("ldi r30, pm_lo8(bits_blue_6)");
	asm("ldi r31, pm_hi8(bits_blue_6)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");

	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_7G)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_7G)) );
	asm("sts %0, r31" : : "X" (TCNT1H));
	asm("sts %0, r30" : : "X" (TCNT1L));
	
	// send 320 cycle pulse
	{
		asm("lds r30, bits_GF + 7");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	// Restore status register, R31, and R30.
	asm("pop r30");
	asm("out 0x3f, r30");
	asm("pop r31");
	asm("pop r30");

	// Done
	asm("reti");
}

//------------------------------------------------------------------------------

__attribute__((naked)) void bits_blue_6() {
	// end previous pulse
	{
		asm("clr r30");
		asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// switch to new sink
	asm("ldi r30, %0" : : "M" (SINK_BLUE));
	asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SINK)) );
	
	// send 3 cycle pulse
	{
		asm("lds r30, bits_BF + 0");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// send 6 cycle pulse
	{
		asm("lds r30, bits_BF + 1");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 3 cycle gap

	asm("nop"); asm("nop"); asm("nop");
	
	// send 11 cycle pulse
	{
		asm("lds r30, bits_BF + 2");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 8 cycle gap

	// save temp registers
	asm("push r25");
	asm("push r28");
	asm("push r29");
		
	asm("nop");
	asm("nop");
	
	// send 20 cycle pulse
	{
		asm("lds r25, bits_BF + 3");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 17 cycle gap
	
	// 14 cycles
	{
		// uint16_t temp = ibright1;
		// if(sample >= tmax1)

		asm("lds r28, sample + 0");
		asm("lds r29, sample + 1");
		asm("lds r30, tmax1 + 0");
		asm("lds r31, tmax1 + 1");
		asm("cp r28, r30");
		asm("cpc r29, r31");
		asm("lds r28, ibright1 + 0");
		asm("lds r29, ibright1 + 1");
	}

	asm("nop"); asm("nop"); asm("nop");
	
	// send 40 cycle pulse
	{
		asm("lds r25, bits_BF + 4");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 37 cycle gap

	// 12 cycles. don't split this block.
	{
		// if(sample >= tmax1)
		//     temp = (temp <= (65535 - BRIGHT1_UP)) ? temp + BRIGHT1_UP : 65535;
		// else
		//     temp = (temp >= BRIGHT1_DOWN) ? temp - BRIGHT1_DOWN : 0;
		asm("brcs signal_low1");
		asm("nop");
	
		asm("signal_high1:");
		asm("ldi r30, %0" : : "M"(lo8(65535 - BRIGHT1_UP)) );
		asm("ldi r31, %0" : : "M"(hi8(65535 - BRIGHT1_UP)) );
		asm("cp r28, r30");
		asm("cpc r29, r31");
		asm("brcc bright_max1");
		asm("subi r28, %0" : : "M"(lo8(-BRIGHT1_UP)) );
		asm("sbci r29, %0" : : "M"(hi8(-BRIGHT1_UP)) );
		asm("nop");
		asm("rjmp store_bright1");

		asm("signal_low1:");
		asm("ldi r30, %0" : : "M"(lo8(BRIGHT1_DOWN)) );
		asm("ldi r31, %0" : : "M"(hi8(BRIGHT1_DOWN)) );
		asm("cp r28, r30");
		asm("cpc r29, r31");
		asm("brcs bright_min1");
		asm("subi r28, %0" : : "M"(lo8(BRIGHT1_DOWN)) );
		asm("sbci r29, %0" : : "M"(hi8(BRIGHT1_DOWN)) );
		asm("nop");
		asm("rjmp store_bright1");

		asm("bright_max1:");
		asm("ser r28");
		asm("ser r29");
		asm("rjmp store_bright1");
	
		asm("bright_min1:");
		asm("clr r28");
		asm("clr r29");
		asm("rjmp store_bright1");
	}
	
	// 4 cycles
	{
		// ibright1 = temp;
		asm("store_bright1:");
		asm("sts ibright1 + 0, r28");
		asm("sts ibright1 + 1, r29");
	}

	// 9 cycles
	{
		// bright1 = pgm_read_byte(exptab+(ibright1 >> 8));
		asm("mov r30, r29");
		asm("ldi r31, 0x00");
		asm("subi r30, lo8(-(exptab))");
		asm("sbci r31, hi8(-(exptab))");
		asm("lpm r30, Z");
		asm("sts bright1, r30");
	}

	// 17 cycles (part 1 of 2, this part is 12 cycles)
	{
		// brightaccum1 += pgm_read_byte(gammatab+bright1);
		asm("ldi r31, 0x00");
		asm("subi r30, lo8(-(gammatab))");
		asm("sbci r31, hi8(-(gammatab))");
		asm("lpm r28, Z");
		asm("ldi r29, 0x00");
		asm("lds r30, brightaccum1 + 0");
		asm("lds r31, brightaccum1 + 1");
		asm("add r30, r28");
	}
	

	// send 80 cycle pulse
	{
		asm("lds r25, bits_BF + 5");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 77 cycle gap

	// (part 2 of 2, this part is 5 cycles)
	{		
		asm("adc r31, r29");
		asm("sts brightaccum1 + 0, r30");
		asm("sts brightaccum1 + 1, r31");
	}
	
	// 14 cycles
	{
		// uint16_t temp = ibright2;
		// if(bass >= tmax2)
		asm("lds r28, bass + 0");
		asm("lds r29, bass + 1");
		asm("lds r30, tmax2 + 0");
		asm("lds r31, tmax2 + 1");
		asm("cp r28, r30");
		asm("cpc r29, r31");
		asm("lds r28, ibright2 + 0");
		asm("lds r29, ibright2 + 1");
	}

	// 12 cycles. don't split this block.
	{
		// if(bass >= tmax2)
		//   temp = (temp <= (65535 - BRIGHT2_UP)) ? temp + BRIGHT2_UP : 65535;
		// else
		//   temp = (temp >= BRIGHT2_DOWN) ? temp - BRIGHT2_DOWN : 0;
		
		asm("brcs signal_low2");
		asm("nop");
	
		asm("signal_high2:");
		asm("ldi r30, %0" : : "M"(lo8(65535 - BRIGHT2_UP)) );
		asm("ldi r31, %0" : : "M"(hi8(65535 - BRIGHT2_UP)) );
		asm("cp r28, r30");
		asm("cpc r29, r31");
		asm("brcc bright_max2");
		asm("subi r28, %0" : : "M"(lo8(-BRIGHT2_UP)) );
		asm("sbci r29, %0" : : "M"(hi8(-BRIGHT2_UP)) );
		asm("nop");
		asm("rjmp store_bright2");

		asm("signal_low2:");
		asm("ldi r30, %0" : : "M"(lo8(BRIGHT2_DOWN)) );
		asm("ldi r31, %0" : : "M"(hi8(BRIGHT2_DOWN)) );
		asm("cp r28, r30");
		asm("cpc r29, r31");
		asm("brcs bright_min2");
		asm("subi r28, %0" : : "M"(lo8(BRIGHT2_DOWN)) );
		asm("sbci r29, %0" : : "M"(hi8(BRIGHT2_DOWN)) );
		asm("nop");
		asm("rjmp store_bright2");

		asm("bright_max2:");
		asm("ser r28");
		asm("ser r29");
		asm("rjmp store_bright2");
	
		asm("bright_min2:");
		asm("clr r28");
		asm("clr r29");
		asm("rjmp store_bright2");
	}

	// 4 cycles
	{
		// ibright2 = temp;
		asm("store_bright2:");
		asm("sts ibright2 + 0, r28");
		asm("sts ibright2 + 1, r29");
	}
	
	// 9 cycles
	{
		// bright2 = pgm_read_byte(exptab+(ibright2 >> 8));
		asm("mov r30, r29");
		asm("ldi r31, 0x00");
		asm("subi r30, lo8(-(exptab))");
		asm("sbci r31, hi8(-(exptab))");
		asm("lpm r30, Z");
		asm("sts bright2, r30");
	}

	// 17 cycles
	{
		// brightaccum2 += pgm_read_byte(gammatab+bright2);
		asm("ldi r31, 0x00");
		asm("subi r30, lo8(-(gammatab))");
		asm("sbci r31, hi8(-(gammatab))");
		asm("lpm r28, Z");
		asm("ldi r29, 0x00");
		asm("lds r30, brightaccum2 + 0");
		asm("lds r31, brightaccum2 + 1");
		asm("add r30, r28");
		asm("adc r31, r29");
		asm("sts brightaccum2 + 0, r30");
		asm("sts brightaccum2 + 1, r31");
	}

	// restore temp registers
	asm("pop r29");
	asm("pop r28");
	// can't restore r25 here, we're out of cycles - do it below.
	// (and after we've updated the button...)

	// set next callback
	asm("ldi r30, pm_lo8(bits_blue_7)");
	asm("ldi r31, pm_hi8(bits_blue_7)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_6B)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_6B)) );
	asm("sts %0, r31" : : "X" (TCNT1H));
	asm("sts %0, r30" : : "X" (TCNT1L));
	
	// send 160 cycle pulse
	{
		asm("lds r30, bits_BF + 6");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	
	// Update button state. 27 cycles + 8 cycles call overhead.
	// Safe to call this from here as it uses r25, r30, r31
	asm("call UpdateButtons");

	// restore r25 out here because the 10 uS block is completely packed.
	asm("pop r25");

	// Restore status register, R31, and R30.
	asm("pop r30");
	asm("out 0x3f, r30");
	asm("pop r31");
	asm("pop r30");

	// Done
	asm("reti");
}	

__attribute__((naked)) void bits_blue_7() {
	// set next callback
	asm("ldi r30, pm_lo8(bits_red_6)");
	asm("ldi r31, pm_hi8(bits_red_6)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");

	// set next timeout
	// 6 cycles
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_7B)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_7B)) );
	asm("sts %0, r31" : : "X" (TCNT1H));
	asm("sts %0, r30" : : "X" (TCNT1L));
	
	// send 320 cycle pulse
	{
		asm("lds r30, bits_BF + 7");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// set blanking flag (3 cycles)
	asm("ldi r30, 1");
	asm("sts blank, r30");
	
	// set ADC start conversion flag
	asm("lds r30, %0" : : "X" (ADCSRA) );
	asm("ori r30, %0" : : "X" (bit(ADSC)) );
	asm("sts %0, r30" : : "X" (ADCSRA) );

	// Restore status register, R31, and R30.
	asm("pop r30");
	asm("out 0x3f, r30");
	asm("pop r31");
	asm("pop r30");

	// Done
	asm("reti");
}	

//------------------------------------------------------------------------------

ISR(TIMER1_OVF_vect, ISR_NAKED)
{
	asm("push r30");
	asm("push r31");
	asm("in r30, 0x3f");
	asm("push r30");
 
	// Call interrupt callback
	asm("lds r30, timer_callback");
	asm("lds r31, timer_callback+1");
	asm("ijmp");
}

//------------------------------------------------------------------------------
// Converts our 8-pixel framebuffer from rgb values to bit planes. If you think
// of each color channel as being an 8x8 matrix of bits, this is basically a
// transpose of that matrix.

// The compiler generates incredibly dumb code for this - the assembly version
// is almost twice as fast.

// Permute and transpose an 8x8 matrix of bits.

__attribute__((naked)) void swap4c(uint8_t* vin, uint8_t* vout) {

	asm("movw r30, r24");
	asm("movw r26, r22");

	asm("ldd r18, z+%0*3" : : "X"(2));
	asm("ldd r19, z+%0*3" : : "X"(0));
	asm("ldd r20, z+%0*3" : : "X"(1));
	asm("ldd r21, z+%0*3" : : "X"(3));

	asm("ldd r22, z+%0*3" : : "X"(4));
	asm("ldd r23, z+%0*3" : : "X"(6));
	asm("ldd r24, z+%0*3" : : "X"(5));
	asm("ldd r25, z+%0*3" : : "X"(7));
	
	asm("ldi r30, 8");

	asm("swaploop:");
	asm("lsr r18"); asm("ror r31");
	asm("lsr r19"); asm("ror r31");
	asm("lsr r20"); asm("ror r31");
	asm("lsr r21"); asm("ror r31");
	asm("lsr r22"); asm("ror r31");
	asm("lsr r23"); asm("ror r31");
	asm("lsr r24"); asm("ror r31");
	asm("lsr r25"); asm("ror r31");
	asm("st x+, r31");
	asm("dec r30");
	asm("brne swaploop");
	asm("ret");
}

void swap() {
	while(blank);
	while(!blank);
	
	swap4c(&pixels[0].r, bits_RF);
	swap4c(&pixels[0].g, bits_GF);
	swap4c(&pixels[0].b, bits_BF);
}

__attribute__((naked)) void clear() {
	asm("sts pixels +  0, r1"); asm("sts pixels +  1, r1"); asm("sts pixels +  2, r1"); 
	asm("sts pixels +  3, r1"); asm("sts pixels +  4, r1"); asm("sts pixels +  5, r1"); 
	asm("sts pixels +  6, r1"); asm("sts pixels +  7, r1"); asm("sts pixels +  8, r1"); 
	asm("sts pixels +  9, r1"); asm("sts pixels + 10, r1"); asm("sts pixels + 11, r1"); 
	asm("sts pixels + 12, r1"); asm("sts pixels + 13, r1"); asm("sts pixels + 14, r1"); 
	asm("sts pixels + 15, r1"); asm("sts pixels + 16, r1"); asm("sts pixels + 17, r1"); 
	asm("sts pixels + 18, r1"); asm("sts pixels + 19, r1"); asm("sts pixels + 20, r1"); 
	asm("sts pixels + 21, r1"); asm("sts pixels + 22, r1"); asm("sts pixels + 23, r1"); 
	asm("ret");
}

//------------------------------------------------------------------------------
// Initialization

void ShutStuffDown() {
	// Disable ports
	
	DDRB = 0x00;
	DDRC = 0x00;
	DDRD = 0x00;
	
	PORTB = 0x00;
	// keep the button pins pulled up so we can come out of sleep mode
	PORTC = (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN);
	PORTD = 0x00;
	
	// Disable analog comparator
	ACSR = bit(ACD);
	//DIDR1 = 0x03;
	
	// Disable all external interrupts.
	EICRA = 0;
	EIMSK = 0;
	PCICR = 0;
	PCMSK0 = 0;
	PCMSK1 = 0;
	PCMSK2 = 0;
	
	// Turn off the SPI interface.
	SPCR = 0;
	
	// Turn off the serial interface, which the bootloader leaves on by default.
	UCSR0A = 0;
	UCSR0B = 0;
	
	// Disable timer 0
	TIMSK0 = 0;
	TCCR0A = 0;
	TCCR0B = 0;

	// Disable timer 1
	TIMSK1 = 0;
	TCCR1A = 0;
	TCCR1B = 0;
	
	// Disable timer 2
	TIMSK2 = 0;
	TCCR2A = 0;
	TCCR2B = 0;
	
	// Disable watchdog
	WDTCSR = 0;
	
	// Power down all peripherals. This has to be done last otherwise some of the
	// above settings don't actually do anything.
	PRR = bit(PRTWI) | bit(PRTIM0) | bit(PRTIM1) | bit(PRTIM2) | bit(PRSPI) | bit(PRUSART0) | bit(PRADC);
}

void SetupLEDs() {
	cli();
	
	ShutStuffDown();
	
	// Port B is our sink port.
	PORTB = 0x00;
	DDRB = 0xFF;

	// Port C is our status port. (Drive C2 high to power the mic on the greenwired board)
	DDRC = (1 << MIC_POWER);
	PORTC = (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << MIC_POWER);

	// Port D is our source port.	
	PORTD = 0x00;
	DDRD = 0xFF;

	timer_callback = bits_red_6;
	
	// Set up ADC to read from channel 1, left-adjust the result, and sample in
	// (14 * 32) = 448 cycles (56 uS @ 8 mhz) - the fastest we can sample and
	// still get 10-bit resolution.

	PRR &= ~bit(PRADC);
	ADMUX  = MIC_PIN | bit(ADLAR);
	ADCSRA = bit(ADEN) | bit(ADPS2) | bit(ADPS0);
	//DIDR0 = 0x3F & ~(1 << BUTTON1_PIN);

	// Set timer 1 to tick at full speed and generate overflow interrupts.
	PRR &= ~bit(PRTIM1);
	TIMSK1 =  (1 << TOIE1);
	TCCR1A = 0;
	TCCR1B = (1 << CS10);
	
	// Device configured, kick off the first ADC conversion and enable
	// interrupts.
	sbi(ADCSRA,ADSC);

	sei();

}

// Update button state. 27 cycles + call overhead. Uses r25, r30, r31
__attribute__((naked)) void UpdateButtons() 
{
	asm("lds r25, buttonstate1");
	asm("sbis %0, %1" : : "I"(_SFR_IO_ADDR(PINC)), "X"(BUTTON1_PIN));
	asm("jmp button_down1");

	asm("button_up1:");
	asm("lds r30, debounce_up1 + 0");
	asm("lds r31, debounce_up1 + 1");
		
	// if(buttonstate1 == 0) debounce_up1 = 0;
	asm("sbrs r25, 0");
	asm("clr r30");
	asm("sbrs r25, 0");
	asm("clr r31");

	// if(debounce_up1 < 0x8000) debounce_up1++;
	asm("sbrs r31, 7");
	asm("subi r30, 0xFF");
	asm("sbrs r31, 7");
	asm("sbci r31, 0xFF");

	// store buttonstate1 & debounce_up1
	asm("ldi r25, 1");
	asm("sts buttonstate1, r25");
	asm("sts debounce_up1 + 0, r30");
	asm("sts debounce_up1 + 1, r31");

	asm("jmp button_done1");

	asm("button_down1:");
	asm("lds r30, debounce_down1 + 0");
	asm("lds r31, debounce_down1 + 1");
		
	// if(buttonstate1 == 1) debounce_down1 = 0;
	asm("sbrc r25, 0");
	asm("clr r30");
	asm("sbrc r25, 0");
	asm("clr r31");

	// if(debounce_down1 < 0x8000) debounce_down1++;
	asm("sbrs r31, 7");
	asm("subi r30, 0xFF");
	asm("sbrs r31, 7");
	asm("sbci r31, 0xFF");

	// store buttonstate1 & debounce_up1
	asm("clr r25");
	asm("sts buttonstate1, r25");
	asm("sts debounce_down1 + 0, r30");
	asm("sts debounce_down1 + 1, r31");

	asm("nop");
	asm("nop");

	asm("button_done1:");

	asm("lds r25, buttonstate2");
	asm("sbis %0, %1" : : "I"(_SFR_IO_ADDR(PINC)), "X"(BUTTON2_PIN));
	asm("jmp button_down2");

	asm("button_up2:");
	asm("lds r30, debounce_up2 + 0");
	asm("lds r31, debounce_up2 + 1");
	
	// if(buttonstate2 == 0) debounce_up2 = 0;
	asm("sbrs r25, 0");
	asm("clr r30");
	asm("sbrs r25, 0");
	asm("clr r31");

	// if(debounce_up2 < 0x8000) debounce_up2++;
	asm("sbrs r31, 7");
	asm("subi r30, 0xFF");
	asm("sbrs r31, 7");
	asm("sbci r31, 0xFF");

	// store buttonstate2 & debounce_up2
	asm("ldi r25, 1");
	asm("sts buttonstate2, r25");
	asm("sts debounce_up2 + 0, r30");
	asm("sts debounce_up2 + 1, r31");

	asm("jmp button_done2");

	asm("button_down2:");
	asm("lds r30, debounce_down2 + 0");
	asm("lds r31, debounce_down2 + 1");
	
	// if(buttonstate2 == 1) debounce_down2 = 0;
	asm("sbrc r25, 0");
	asm("clr r30");
	asm("sbrc r25, 0");
	asm("clr r31");

	// if(debounce_down2 < 0x8000) debounce_down2++;
	asm("sbrs r31, 7");
	asm("subi r30, 0xFF");
	asm("sbrs r31, 7");
	asm("sbci r31, 0xFF");

	// store buttonstate2 & debounce_up2
	asm("clr r25");
	asm("sts buttonstate2, r25");
	asm("sts debounce_down2 + 0, r30");
	asm("sts debounce_down2 + 1, r31");

	asm("nop");
	asm("nop");

	asm("button_done2:");

	asm("ret");
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

// watchdog interrupt has to exist or the chip resets.
ISR(WDT_vect) {}

void GoToSleep()
{
	uint8_t cursor = 0;
	uint8_t column = 0;

	ShutStuffDown();
	sleep_bod_disable();

	SMCR = bit(SE) | bit(SM1);
	WDTCSR = bit(WDCE) | bit(WDE);
	WDTCSR = bit(WDIE);
	
	DDRD = 0xFF;
	PORTD = 0x00;
	DDRB = 0xFF;
	PORTB = SINK_GREEN;
	
	while(1)
	{
		asm("sleep");
		
		UpdateButtons();
		
		if((buttonstate1 == 0) && (debounce_down1 > 60))
		{
			SMCR = 0;
			WDTCSR = bit(WDCE) | bit(WDE);
			WDTCSR = 0;
			SetupLEDs();
			return;
		}

		uint8_t bright = pgm_read_byte(gammasin2 + cursor) >> 2;
		if(bright) 
		{
			PORTD = pgm_read_byte(sources + column);
			for(uint8_t i = 0; i < bright; i++) { asm("nop"); }
			PORTD = 0;
		}
	
		cursor += 1;
		if(cursor == 0) column = (column == 13) ? 0 : column + 1;
	}
}
