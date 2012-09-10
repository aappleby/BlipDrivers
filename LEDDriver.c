#include "LEDDriver.h"
#include "Defines.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h> 

void bits_red_6();
void bits_red_7();
	
void bits_green_6();
void bits_green_7();
	
void bits_blue_6();
void bits_blue_7();

// Front buffer
uint8_t bits_RF[8];
uint8_t bits_GF[8];
uint8_t bits_BF[8];

// Back buffer
uint8_t r[8];
uint8_t g[8];
uint8_t b[8];

uint16_t led_tick = 0;
volatile uint8_t blank = 0;

void(*timer_callback)() ;

//------------------------------------------------------------------------------
// New interrupt handlers

#define TIMEOUT_6R (65536 - 121)
#define TIMEOUT_6G (65536 - 121)
#define TIMEOUT_6B (65536 - 121)

#define TIMEOUT_7R (65536 - 294)
#define TIMEOUT_7G (65536 - 294)
#define TIMEOUT_7B (65536 - 294)

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
	
	// send 0.375 uS pulse
	{
		asm("lds r30, bits_RF + 0");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// send 0.625 uS pulse
	{
		asm("lds r30, bits_RF + 1");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	
	// 1 cycle gap

	// set the monitor bit (2 cycles)
	asm("sbi %0, 2" : : "I"(_SFR_IO_ADDR(PORTC)) );
	
	// send 1.25 uS pulse
	{
		asm("lds r30, bits_RF + 2");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 7 cycle gap
	
	// save temp registers
	asm("push r25");
	asm("push r28");
	asm("push r29");
	asm("nop");
	
	// send 2.5 uS pulse
	{
		asm("lds r25, bits_RF + 3");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 17 cycle gap
	
	asm("push r26");
	asm("push r27");
	
	// remove dc bias, part 1
	{
		// sample -= accumD;
		// accumD += (sample >> DCFILTER);

		asm("lds r28, sample + 0");
		asm("lds r29, sample + 1");
		asm("lds r30, accumD + 0");
		asm("lds r31, accumD + 1");
	
		asm("sub r28, r30");
		asm("sbc r29, r31");
	
		asm("movw r26, r28");
		asm("asr r27"); asm("ror r26");
	}		

	// send 5.0 uS pulse
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
	
	{
		// send 10.0 uS pulse
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
	{	
		asm("pop r27");
		asm("pop r26");
	}		

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
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));

	// send 20.0 uS pulse
	{
		asm("lds r30, bits_RF + 6");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	asm("ret");
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
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));

	// send 40.0 uS pulse
	{
		asm("lds r30, bits_RF + 7");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	asm("ret");
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
	
	// send 0.375 uS pulse
	{
		asm("lds r30, bits_GF + 0");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// send 0.625 uS pulse
	{
		asm("lds r30, bits_GF + 1");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 1 cycle gap
	
	// clear the monitor bit (2 cycles)
	asm("cbi %0, 2" : : "I"(_SFR_IO_ADDR(PORTC)) );
	
	// send 1.25 uS pulse
	{
		asm("lds r30, bits_GF + 2");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 7 cycle gap
	
	// save temp registers
	
	asm("push r25");
	asm("push r28");
	asm("push r29");
	asm("nop");
	
	// send 2.5 uS pulse
	{
		asm("lds r25, bits_GF + 3");
		asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop");
	
	// send 5.0 uS pulse
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
		asm("clr r25");
		asm("cpi r30, 60");
		asm("cpc r31, r25"); // r25 is a zero register
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
		asm("clr r25");
		asm("cpi r30, 60");
		asm("cpc r31, r25"); // r25 is a zero register
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

	// send 10.0 uS pulse
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

	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
	asm("nop"); asm("nop");
	
	// restore temp registers
	asm("pop r29");
	asm("pop r28");
	asm("pop r25");
	
	// set next callback
	asm("ldi r30, pm_lo8(bits_green_7)");
	asm("ldi r31, pm_hi8(bits_green_7)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_6G)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_6G)) );
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));
	
	// send 20.0 uS pulse
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
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_7G)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_7G)) );
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));
	
	// send 40.0 uS pulse
	{
		asm("lds r30, bits_GF + 7");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	asm("ret");
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
	
	// send 0.375 uS pulse
	{
		asm("lds r30, bits_BF + 0");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}		
	
	// send 0.625 uS pulse
	{
		asm("lds r30, bits_BF + 1");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 2 cycle gap

	asm("nop"); asm("nop");
	
	// send 1.25 uS pulse
	{
		asm("lds r30, bits_BF + 2");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	// 7 cycle gap

	// save temp registers
	asm("push r25");
	asm("push r28");
	asm("push r29");
		
	asm("nop");
	
	// send 2.5 uS pulse
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
	
	// send 5.0 uS pulse
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
	

	// send 10.0 uS pulse
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

	// set next callback
	asm("ldi r30, pm_lo8(bits_blue_7)");
	asm("ldi r31, pm_hi8(bits_blue_7)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(TIMEOUT_6B)) );
	asm("ldi r31, %0" : : "M" (hi8(TIMEOUT_6B)) );
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));
	
	// send 20.0 uS pulse
	{
		asm("lds r30, bits_BF + 6");
		asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
	}
	
	// restore r25 out here because the 10 uS block is completely packed.
	asm("pop r25");

	asm("ret");
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
	asm("sts %0, r31" : : "" (TCNT1H));
	asm("sts %0, r30" : : "" (TCNT1L));
	
	// send 40.0 uS pulse
	{
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
	asm("push r25");
	asm("push r28");
	asm("push r29");
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
	asm("pop r29");
	asm("pop r28");
	asm("pop r25");

	// Done
	asm("reti");
}

//------------------------------------------------------------------------------
// Framebuffer conversion

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