#include "Config.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h> 

#define lo8(A) (uint16_t(A) & 0xFF)
#define hi8(A) (uint16_t(A) >> 8)

extern "C" {
	void bits_red_6();
	void bits_red_7();
	
	void bits_green_6();
	void bits_green_7();
	
	void bits_blue_6();
	void bits_blue_7();
};

// Front buffer
uint8_t bits_RF[8];
uint8_t bits_GF[8];
uint8_t bits_BF[8];

uint16_t led_tick = 0;
volatile uint8_t blank = 0;

//------------------------------------------------------------------------------
// New interrupt handlers

#define TIMEOUT_6 (65536 - 127)
#define TIMEOUT_7 (65536 - 300)

__attribute__((naked)) void bits_red_6() {
	// end previous pulse
	asm("clr r30");
	asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SOURCE)) );

	// Dead space between PWM cycles. Time spent here should cause
	// the overall PWM rate to be 4.096 khz, or ~1953 cycles.
	
	{
		// clear blanking flag (2 cycles)
		asm("sts blank, r30");
	
		// increment LED tick (10 cycles)
		asm("lds r30, led_tick");
		asm("lds r31, led_tick+1");
		asm("adiw r30, 1");
		asm("sts led_tick, r30");
		asm("sts led_tick+1, r31");

		// copy adc sample to sample buffer
		asm("lds r30, %0" : : "" (ADCL));
		asm("lds r31, %0" : : "" (ADCH));
		asm("sts sample + 0, r30");
		asm("sts sample + 1, r31");

		asm("nop"); asm("nop");
	}		

	// switch to new sink
	asm("ldi r30, %0" : : "M" (SINK_RED));
	asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SINK)) );
	
	{
		// send 0.375 uS pulse
		asm("lds r30, bits_RF + 0");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	{
		// send 0.625 uS pulse
		asm("lds r30, bits_RF + 1");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	// set the monitor bit (2 cycles)
	asm("sbi %0, 2" : : "I"(_SFR_IO_ADDR(PORTC)) );
	
	{
		// send 1.25 uS pulse
		asm("lds r30, bits_RF + 2");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// save additional temp registers
	asm("push r25");
	asm("push r26");
	asm("push r27");
	asm("nop");
	
	{
		// send 2.5 uS pulse
		asm("lds r25, bits_RF + 3");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	asm("push r28");
	asm("push r29");

	// sample -= accumD;
	// accumD += (sample >> DCFILTER);

	asm("lds r26, sample + 0");
	asm("lds r27, sample + 1");
	asm("lds r30, accumD + 0");
	asm("lds r31, accumD + 1");
	
	asm("sub r26, r30");
	asm("sbc r27, r31");
	
	asm("movw r28, r26");
	asm("asr r29"); asm("ror r28");
	
	{
		// send 5.0 uS pulse		
		asm("lds r25, bits_RF + 4");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	asm("asr r29"); asm("ror r28");
	asm("asr r29"); asm("ror r28");
	asm("asr r29"); asm("ror r28");
	asm("asr r29"); asm("ror r28");
	
	asm("add r30, r28");
	asm("adc r31, r29");
	
	asm("sts accumD + 0, r30");
	asm("sts accumD + 1, r31");
	
	// sample -= accumB;
	// accumB += (sample >> BASSFILTER);
	
	asm("lds r30, accumB + 0");
	asm("lds r31, accumB + 1");
	asm("sub r26, r30");
	asm("sbc r27, r31");
	
	asm("movw r28, r26");
	asm("asr r29"); asm("ror r28");
	asm("asr r29"); asm("ror r28");
	asm("asr r29"); asm("ror r28");
	
	asm("add r30, r28");
	asm("adc r31, r29");
	
	asm("sts accumB + 0, r30");
	asm("sts accumB + 1, r31");
	
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	
	{
		// send 10.0 uS pulse
		asm("lds r25, bits_RF + 5");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	
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
	
	// if(sample < 0) sample = -sample;
	
	asm("sbrc 27, 7");
	asm("jmp negate_treb");
	asm("nop");
	asm("jmp treb_done");
	asm("negate_treb:");
	asm("com r27");
	asm("neg r26");
	asm("sbci r27, 0xFF");
	asm("treb_done:");
	
	asm("sts sample + 0, r26");
	asm("sts sample + 1, r27");
	
	// tickcount += 4;
	
	asm("lds r30, tickcount");
	asm("subi r30, 0xFC");
	asm("sts tickcount, r30");
	
	// if(tmax1 & 0x8000) tmax1 -= 256;
	// if(tmax1 < TRIG1_CLAMP) tmax1++;
	
	asm("lds r30, tmax1 + 0");
	asm("lds r31, tmax1 + 1");

	// Clamp if above 32767
	asm("sbrc r31, 7");
	asm("subi r31, 0x01");
	
	// Clamp if below 60
	asm("clr r25");
	asm("cpi r30, 60");
	asm("cpc r31, r25"); // r25 is a zero register
	asm("brge trig1_noclamp");
	asm("adiw r30, 1");
	asm("rjmp trig1_clampdone");
	
	asm("trig1_noclamp:");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("trig1_clampdone:");
	
	asm("sts tmax1 + 0, r30");
	asm("sts tmax1 + 1, r31");
	
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");

	// restore temp registers
	asm("pop r29");
	asm("pop r28");
	asm("pop r27");
	asm("pop r26");
	asm("pop r25");
	
	// set next callback
	asm("ldi r30, pm_lo8(bits_red_7)");
	asm("ldi r31, pm_hi8(bits_red_7)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_6)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_6)) );
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));

	{
		// send 20.0 uS pulse
		asm("lds r30, bits_RF + 6");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	asm("ret");
}

__attribute__((naked)) void bits_red_7() {
	// set next callback
	asm("ldi r30, pm_lo8(bits_green_6)");
	asm("ldi r31, pm_hi8(bits_green_6)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");

	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_7)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_7)) );
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));

	// send 40.0 uS pulse
	asm("lds r30, bits_RF + 7");
	asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	
	asm("ret");
}

//------------------------------------------------------------------------------

__attribute__((naked)) void bits_green_6() {
	// end previous pulse
	asm("clr r30");
	asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SOURCE)) );

	// switch to new sink
	asm("ldi r30, %0" : : "M" (SINK_GREEN));
	asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SINK)) );
	
	// send 0.375 uS pulse
	{
		asm("lds r31, bits_GF + 0");
		asm("out %0, r31" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// send 0.625 uS pulse
	{
		asm("lds r30, bits_GF + 1");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// clear the monitor bit (2 cycles)
	asm("cbi %0, 2" : : "I"(_SFR_IO_ADDR(PORTC)) );
	
	// send 1.25 uS pulse
	{
		asm("lds r30, bits_GF + 2");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// save additional temp registers
	asm("push r25");
	asm("push r26");
	asm("push r27");
	asm("nop");
	
	// send 2.5 uS pulse
	{
		asm("lds r25, bits_GF + 3");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	asm("push r28");
	asm("push r29");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop");
	
	// send 5.0 uS pulse
	{
		asm("lds r25, bits_GF + 4");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop");

	// send 10.0 uS pulse
	{
		asm("lds r25, bits_GF + 5");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	
	// restore temp registers
	asm("pop r29");
	asm("pop r28");
	asm("pop r27");
	asm("pop r26");
	asm("pop r25");

	// set next callback
	asm("ldi r30, pm_lo8(bits_green_7)");
	asm("ldi r31, pm_hi8(bits_green_7)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_6)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_6)) );
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));

	// send 10.0 uS pulse
	{
		asm("lds r30, bits_GF + 6");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	asm("ret");
}

__attribute__((naked)) void bits_green_7() {
	// set next callback
	asm("ldi r30, pm_lo8(bits_blue_6)");
	asm("ldi r31, pm_hi8(bits_blue_6)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");

	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_7)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_7)) );
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));
	
	// send 40.0 uS pulse
	asm("lds r30, bits_GF + 7");
	asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	
	asm("ret");
}

//------------------------------------------------------------------------------

__attribute__((naked)) void bits_blue_6() {
	// end previous pulse
	asm("clr r30");
	asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SOURCE)) );
	
	// switch to new sink
	asm("ldi r30, %0" : : "M" (SINK_BLUE));
	asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SINK)) );
	
	{
		// send 0.375 uS pulse
		asm("lds r30, bits_BF + 0");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	{
		// send 0.625 uS pulse
		asm("lds r30, bits_BF + 1");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	asm("nop"); asm("nop");
	
	{
		// send 1.25 uS pulse
		asm("lds r30, bits_BF + 2");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	
	// save additional temp registers
	asm("push r28");
	asm("push r29");
	asm("nop"); asm("nop"); asm("nop");
	
	{
		// send 2.5 uS pulse
		asm("lds r30, bits_BF + 3");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
			
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop");
	
	{
		// send 5.0 uS pulse
		asm("lds r30, bits_BF + 4");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
			
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop");

	{
		// send 10.0 uS pulse
		asm("lds r30, bits_BF + 5");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
			
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop");
	
	// restore temp registers
	asm("pop r29");
	asm("pop r28");

	// set next callback
	asm("ldi r30, pm_lo8(bits_blue_7)");
	asm("ldi r31, pm_hi8(bits_blue_7)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_6)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_6)) );
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));

	{
		// send 20.0 uS pulse	
		asm("lds r30, bits_BF + 6");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	asm("ret");
}	

__attribute__((naked)) void bits_blue_7() {
	// set next callback
	asm("ldi r30, pm_lo8(bits_red_6)");
	asm("ldi r31, pm_hi8(bits_red_6)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");

	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_7)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_7)) );
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));
	
	{
		// send 40.0 uS pulse
		asm("lds r30, bits_BF + 7");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// set blanking flag (3 cycles)
	asm("ldi r30, 1");
	asm("sts blank, r30");
	
	// set ADC start conversion flag
	asm("lds r30, %0" : : "" (ADCSRA));
	asm("ori r30, 0x40");
	asm("sts %0, r30" : : "" (ADCSRA));
	
	asm("ret");
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
	asm("icall");

	// Restore status register, R31, and R30.
	asm("pop r30");
	asm("out 0x3f, r30");
	asm("pop r31");
	asm("pop r30");

	// Done
	asm("reti");
}

//------------------------------------------------------------------------------
// Framebuffer conversion

uint8_t r[8];
uint8_t g[8];
uint8_t b[8];

// Conversion from rgb to bitpacked buffer takes ~581 cycles. It should be more
// like 480, but this isn't really worth optimizing further.

void swap() {
	
	uint8_t c0 = r[0], c1 = r[1], c2 = r[2], c3 = r[3], c4 = r[4], c5 = r[5], c6 = r[6], c7 = r[7];
	uint8_t t = 0;
	
	if(c0 & 0x80) t |= SOURCE_1; if(c1 & 0x80) t |= SOURCE_2; if(c2 & 0x80) t |= SOURCE_3; if(c3 & 0x80) t |= SOURCE_4;
	if(c4 & 0x80) t |= SOURCE_5; if(c5 & 0x80) t |= SOURCE_6; if(c6 & 0x80) t |= SOURCE_7; if(c7 & 0x80) t |= SOURCE_8;
	bits_RF[7] = t;
		
	t = 0;
	if(c0 & 0x40) t |= SOURCE_1; if(c1 & 0x40) t |= SOURCE_2; if(c2 & 0x40) t |= SOURCE_3; if(c3 & 0x40) t |= SOURCE_4;
	if(c4 & 0x40) t |= SOURCE_5; if(c5 & 0x40) t |= SOURCE_6; if(c6 & 0x40) t |= SOURCE_7; if(c7 & 0x40) t |= SOURCE_8;
	bits_RF[6] = t;
		
	t = 0;
	if(c0 & 0x20) t |= SOURCE_1; if(c1 & 0x20) t |= SOURCE_2; if(c2 & 0x20) t |= SOURCE_3; if(c3 & 0x20) t |= SOURCE_4;
	if(c4 & 0x20) t |= SOURCE_5; if(c5 & 0x20) t |= SOURCE_6; if(c6 & 0x20) t |= SOURCE_7; if(c7 & 0x20) t |= SOURCE_8;
	bits_RF[5] = t;

	t = 0;
	if(c0 & 0x10) t |= SOURCE_1; if(c1 & 0x10) t |= SOURCE_2; if(c2 & 0x10) t |= SOURCE_3; if(c3 & 0x10) t |= SOURCE_4;
	if(c4 & 0x10) t |= SOURCE_5; if(c5 & 0x10) t |= SOURCE_6; if(c6 & 0x10) t |= SOURCE_7; if(c7 & 0x10) t |= SOURCE_8;
	bits_RF[4] = t;

	t = 0;
	if(c0 & 0x08) t |= SOURCE_1; if(c1 & 0x08) t |= SOURCE_2; if(c2 & 0x08) t |= SOURCE_3; if(c3 & 0x08) t |= SOURCE_4;
	if(c4 & 0x08) t |= SOURCE_5; if(c5 & 0x08) t |= SOURCE_6; if(c6 & 0x08) t |= SOURCE_7; if(c7 & 0x08) t |= SOURCE_8;
	bits_RF[3] = t;

	t = 0;
	if(c0 & 0x04) t |= SOURCE_1; if(c1 & 0x04) t |= SOURCE_2; if(c2 & 0x04) t |= SOURCE_3; if(c3 & 0x04) t |= SOURCE_4;
	if(c4 & 0x04) t |= SOURCE_5; if(c5 & 0x04) t |= SOURCE_6; if(c6 & 0x04) t |= SOURCE_7; if(c7 & 0x04) t |= SOURCE_8;
	bits_RF[2] = t;
		
	t = 0;
	if(c0 & 0x02) t |= SOURCE_1; if(c1 & 0x02) t |= SOURCE_2; if(c2 & 0x02) t |= SOURCE_3; if(c3 & 0x02) t |= SOURCE_4;
	if(c4 & 0x02) t |= SOURCE_5; if(c5 & 0x02) t |= SOURCE_6; if(c6 & 0x02) t |= SOURCE_7; if(c7 & 0x02) t |= SOURCE_8;
	bits_RF[1] = t;

	t = 0;
	if(c0 & 0x01) t |= SOURCE_1; if(c1 & 0x01) t |= SOURCE_2; if(c2 & 0x01) t |= SOURCE_3; if(c3 & 0x01) t |= SOURCE_4;
	if(c4 & 0x01) t |= SOURCE_5; if(c5 & 0x01) t |= SOURCE_6; if(c6 & 0x01) t |= SOURCE_7; if(c7 & 0x01) t |= SOURCE_8;
	bits_RF[0] = t;
	
	c0 = g[0]; c1 = g[1]; c2 = g[2]; c3 = g[3]; c4 = g[4]; c5 = g[5]; c6 = g[6]; c7 = g[7];
	
	t = 0;
	if(c0 & 0x80) t |= SOURCE_1; if(c1 & 0x80) t |= SOURCE_2; if(c2 & 0x80) t |= SOURCE_3; if(c3 & 0x80) t |= SOURCE_4;
	if(c4 & 0x80) t |= SOURCE_5; if(c5 & 0x80) t |= SOURCE_6; if(c6 & 0x80) t |= SOURCE_7; if(c7 & 0x80) t |= SOURCE_8;
	bits_GF[7] = t;
		
	t = 0;
	if(c0 & 0x40) t |= SOURCE_1; if(c1 & 0x40) t |= SOURCE_2; if(c2 & 0x40) t |= SOURCE_3; if(c3 & 0x40) t |= SOURCE_4;
	if(c4 & 0x40) t |= SOURCE_5; if(c5 & 0x40) t |= SOURCE_6; if(c6 & 0x40) t |= SOURCE_7; if(c7 & 0x40) t |= SOURCE_8;
	bits_GF[6] = t;
		
	t = 0;
	if(c0 & 0x20) t |= SOURCE_1; if(c1 & 0x20) t |= SOURCE_2; if(c2 & 0x20) t |= SOURCE_3; if(c3 & 0x20) t |= SOURCE_4;
	if(c4 & 0x20) t |= SOURCE_5; if(c5 & 0x20) t |= SOURCE_6; if(c6 & 0x20) t |= SOURCE_7; if(c7 & 0x20) t |= SOURCE_8;
	bits_GF[5] = t;

	t = 0;
	if(c0 & 0x10) t |= SOURCE_1; if(c1 & 0x10) t |= SOURCE_2; if(c2 & 0x10) t |= SOURCE_3; if(c3 & 0x10) t |= SOURCE_4;
	if(c4 & 0x10) t |= SOURCE_5; if(c5 & 0x10) t |= SOURCE_6; if(c6 & 0x10) t |= SOURCE_7; if(c7 & 0x10) t |= SOURCE_8;
	bits_GF[4] = t;

	t = 0;
	if(c0 & 0x08) t |= SOURCE_1; if(c1 & 0x08) t |= SOURCE_2; if(c2 & 0x08) t |= SOURCE_3; if(c3 & 0x08) t |= SOURCE_4;
	if(c4 & 0x08) t |= SOURCE_5; if(c5 & 0x08) t |= SOURCE_6; if(c6 & 0x08) t |= SOURCE_7; if(c7 & 0x08) t |= SOURCE_8;
	bits_GF[3] = t;

	t = 0;
	if(c0 & 0x04) t |= SOURCE_1; if(c1 & 0x04) t |= SOURCE_2; if(c2 & 0x04) t |= SOURCE_3; if(c3 & 0x04) t |= SOURCE_4;
	if(c4 & 0x04) t |= SOURCE_5; if(c5 & 0x04) t |= SOURCE_6; if(c6 & 0x04) t |= SOURCE_7; if(c7 & 0x04) t |= SOURCE_8;
	bits_GF[2] = t;
		
	t = 0;
	if(c0 & 0x02) t |= SOURCE_1; if(c1 & 0x02) t |= SOURCE_2; if(c2 & 0x02) t |= SOURCE_3; if(c3 & 0x02) t |= SOURCE_4;
	if(c4 & 0x02) t |= SOURCE_5; if(c5 & 0x02) t |= SOURCE_6; if(c6 & 0x02) t |= SOURCE_7; if(c7 & 0x02) t |= SOURCE_8;
	bits_GF[1] = t;

	t = 0;
	if(c0 & 0x01) t |= SOURCE_1; if(c1 & 0x01) t |= SOURCE_2; if(c2 & 0x01) t |= SOURCE_3; if(c3 & 0x01) t |= SOURCE_4;
	if(c4 & 0x01) t |= SOURCE_5; if(c5 & 0x01) t |= SOURCE_6; if(c6 & 0x01) t |= SOURCE_7; if(c7 & 0x01) t |= SOURCE_8;
	bits_GF[0] = t;
	
	c0 = b[0]; c1 = b[1]; c2 = b[2]; c3 = b[3]; c4 = b[4]; c5 = b[5]; c6 = b[6]; c7 = b[7];
	
	t = 0;
	if(c0 & 0x80) t |= SOURCE_1; if(c1 & 0x80) t |= SOURCE_2; if(c2 & 0x80) t |= SOURCE_3; if(c3 & 0x80) t |= SOURCE_4;
	if(c4 & 0x80) t |= SOURCE_5; if(c5 & 0x80) t |= SOURCE_6; if(c6 & 0x80) t |= SOURCE_7; if(c7 & 0x80) t |= SOURCE_8;
	bits_BF[7] = t;
		
	t = 0;
	if(c0 & 0x40) t |= SOURCE_1; if(c1 & 0x40) t |= SOURCE_2; if(c2 & 0x40) t |= SOURCE_3; if(c3 & 0x40) t |= SOURCE_4;
	if(c4 & 0x40) t |= SOURCE_5; if(c5 & 0x40) t |= SOURCE_6; if(c6 & 0x40) t |= SOURCE_7; if(c7 & 0x40) t |= SOURCE_8;
	bits_BF[6] = t;
		
	t = 0;
	if(c0 & 0x20) t |= SOURCE_1; if(c1 & 0x20) t |= SOURCE_2; if(c2 & 0x20) t |= SOURCE_3; if(c3 & 0x20) t |= SOURCE_4;
	if(c4 & 0x20) t |= SOURCE_5; if(c5 & 0x20) t |= SOURCE_6; if(c6 & 0x20) t |= SOURCE_7; if(c7 & 0x20) t |= SOURCE_8;
	bits_BF[5] = t;

	t = 0;
	if(c0 & 0x10) t |= SOURCE_1; if(c1 & 0x10) t |= SOURCE_2; if(c2 & 0x10) t |= SOURCE_3; if(c3 & 0x10) t |= SOURCE_4;
	if(c4 & 0x10) t |= SOURCE_5; if(c5 & 0x10) t |= SOURCE_6; if(c6 & 0x10) t |= SOURCE_7; if(c7 & 0x10) t |= SOURCE_8;
	bits_BF[4] = t;

	t = 0;
	if(c0 & 0x08) t |= SOURCE_1; if(c1 & 0x08) t |= SOURCE_2; if(c2 & 0x08) t |= SOURCE_3; if(c3 & 0x08) t |= SOURCE_4;
	if(c4 & 0x08) t |= SOURCE_5; if(c5 & 0x08) t |= SOURCE_6; if(c6 & 0x08) t |= SOURCE_7; if(c7 & 0x08) t |= SOURCE_8;
	bits_BF[3] = t;

	t = 0;
	if(c0 & 0x04) t |= SOURCE_1; if(c1 & 0x04) t |= SOURCE_2; if(c2 & 0x04) t |= SOURCE_3; if(c3 & 0x04) t |= SOURCE_4;
	if(c4 & 0x04) t |= SOURCE_5; if(c5 & 0x04) t |= SOURCE_6; if(c6 & 0x04) t |= SOURCE_7; if(c7 & 0x04) t |= SOURCE_8;
	bits_BF[2] = t;
		
	t = 0;
	if(c0 & 0x02) t |= SOURCE_1; if(c1 & 0x02) t |= SOURCE_2; if(c2 & 0x02) t |= SOURCE_3; if(c3 & 0x02) t |= SOURCE_4;
	if(c4 & 0x02) t |= SOURCE_5; if(c5 & 0x02) t |= SOURCE_6; if(c6 & 0x02) t |= SOURCE_7; if(c7 & 0x02) t |= SOURCE_8;
	bits_BF[1] = t;

	t = 0;
	if(c0 & 0x01) t |= SOURCE_1; if(c1 & 0x01) t |= SOURCE_2; if(c2 & 0x01) t |= SOURCE_3; if(c3 & 0x01) t |= SOURCE_4;
	if(c4 & 0x01) t |= SOURCE_5; if(c5 & 0x01) t |= SOURCE_6; if(c6 & 0x01) t |= SOURCE_7; if(c7 & 0x01) t |= SOURCE_8;
	bits_BF[0] = t;
}	

//------------------------------------------------------------------------------
// Initialization

void SetupLEDs() {
	PORTD = 0x00;
	DDRD = 0xFF;
	PORT_SINK = SINK_BLUE;
	PORTB = 0x00;

	timer_callback = bits_red_6;
	
	cli();
	TIMSK1 =  (1 << TOIE1);
	TCCR1A = 0;
	TCCR1B = (1 << CS10);
	sei();
}