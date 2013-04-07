#include "LEDDriver.h"
#include "Bits.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h> 
#include <avr/sleep.h>
#include <util/delay.h>

//-----------------------------------------------------------------------------
// Externally-visible data.

// Back buffer, standard RGB format.
struct Pixel pixels[8];

// PWM cycle tick, 4.096 kilohertz.
uint32_t volatile blip_tick;

// Output brightness, treble channel
uint8_t bright1 = 0;

// Output brightness, bass channel
uint8_t bright2 = 0;

// Button debounce counters
volatile uint8_t buttonstate1 = 0;
volatile uint16_t debounce_up1 = 0;
volatile uint16_t debounce_down1 = 0;

volatile uint8_t buttonstate2 = 0;
volatile uint16_t debounce_up2 = 0;
volatile uint16_t debounce_down2 = 0;

//-----------------------------------------------------------------------------
// Internal data

// Front buffer, bit-planes format.
uint8_t bits_RF[8];
uint8_t bits_GF[8];
uint8_t bits_BF[8];

// Blanking interval flag.
volatile uint8_t blank;

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

// Brightness accumulator, treble channel.
uint16_t brightaccum1 = 0;

// Brightness accumulator, bass channel.
uint16_t brightaccum2 = 0;

// Internal tick count to trigger volume adaptation every 64 cycles.
uint8_t tickcount = 0;

// PWM callback function pointer dispatched by the timer interrupt.
void (*timer_callback)() ;

//------------------------------------------------------------------------------
// Timer interrupt, dispatches our LED update callback. Note that we _ijmp_ to
// the callback, and _reti_ from the callback - this saves a few cycles per
// interrupt, which ends up being a big win when we're doing 24000 of them per
// second.

ISR(TIMER1_OVF_vect, ISR_NAKED)
{
  // We're clobbering R30 and R31, so we are required to save them.
  asm("push r30");
  asm("push r31");
  
  // We also have to save the status register.
  asm("in r30, 0x3f");
  asm("push r30");
  
  // Call interrupt callback
  asm("lds r30, timer_callback");
  asm("lds r31, timer_callback+1");
  asm("ijmp");
}

//------------------------------------------------------------------------------
// Interrupt handlers

// Each callback needs to fire a precise number of cycles after the previous
// callback finished. These constants represent the amount of delay needed
// between adjacent callbacks so that everything stays in sync. Since our timer
// is counting up to 65536, we represent a delay of N as 65536 - N.

#define RED_FIELD_A_TIMEOUT   (65536 - 127)
#define RED_FIELD_B_TIMEOUT   (65536 - 300)

#define GREEN_FIELD_A_TIMEOUT (65536 - 127)
#define GREEN_FIELD_B_TIMEOUT (65536 - 300)

#define BLUE_FIELD_A_TIMEOUT  (65536 - 127)
#define BLUE_FIELD_B_TIMEOUT  (65536 - 300)

// Our PWM period is divided into 3 fields for red/green/blue, each of which is
// in turn divided into 2 callbacks for the 'low' bits 0-6 and the 'high' bit 7.
// We interleave our audio processing code between the LED pulses for bits 0-5,
// and we release the CPU back to the user after sending pulses for bits 6 and 7.

// Each field is 3+5+10+20+40+80+160+320 = 638 cycles long, and we have 3 of them
// per period - that's 1914 cycles.
// Our CPU runs at 8 mhz, and we want a refresh rate of 4096 hz - that's 1953
// cycles. We lose a few cycles in overhead between fields, but we still end up
// with ~20 cycles of dead space each period. We could probably do something more
// useful there.

// Note: absolutely everything in these callbacks _must_ run in _exactly_ the same
// number of cycles - all possible paths through branching code must be timed and
// padded with NOPs to keep the branch lengths equal.

// Sends pulses for red field bits 0 - 6 to the LEDs, interleaving those pulses
// with our audio processing code. Since this is the first callback fired for
// a PWM period, we also do a small amount of additional bookkeeping.

__attribute__((naked)) void red_field_A() {
	// end previous pulse
	{
		asm("clr r30");
		asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SOURCE)) );
	}		

	// 20 cycle dead space between PWM cycles. Time spent here should cause
	// the overall PWM rate to be 4.096 khz, or ~1953 cycles.
	
	{
		// clear blanking flag (2 cycles)
		asm("sts blank, r30");

		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
		asm("nop"); asm("nop");
    asm("nop");
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
    
    // Cycle padding if we're not doing adaptation this period.
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
	asm("ldi r30, pm_lo8(red_field_B)");
	asm("ldi r31, pm_hi8(red_field_B)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout, 6 cycles
	asm("ldi r30, %0" : : "M" (lo8(RED_FIELD_A_TIMEOUT)) );
	asm("ldi r31, %0" : : "M" (hi8(RED_FIELD_A_TIMEOUT)) );
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

//------------------------------------------------------------------------------
// Sends red field bit 7 pulse and queues callback for green field.

__attribute__((naked)) void red_field_B() {
	// set next callback
	asm("ldi r30, pm_lo8(green_field_A)");
	asm("ldi r31, pm_hi8(green_field_A)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");

	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(RED_FIELD_B_TIMEOUT)) );
	asm("ldi r31, %0" : : "M" (hi8(RED_FIELD_B_TIMEOUT)) );
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
// Green field A. Pulses and audio.

__attribute__((naked)) void green_field_A() {
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
		asm("lds r28, blip_tick+0");
		asm("lds r29, blip_tick+1");
		asm("lds r30, blip_tick+2");
		asm("lds r31, blip_tick+3");
		asm("adiw r28, 0x01");
		asm("adc r30, r25");
		asm("adc r31, r25");
		asm("sts blip_tick+0, r28");
		asm("sts blip_tick+1, r29");
		asm("sts blip_tick+2, r30");
		asm("sts blip_tick+3, r31");
	}
	asm("nop"); asm("nop");
	
	// set next callback
	asm("ldi r30, pm_lo8(green_field_B)");
	asm("ldi r31, pm_hi8(green_field_B)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(GREEN_FIELD_A_TIMEOUT)) );
	asm("ldi r31, %0" : : "M" (hi8(GREEN_FIELD_A_TIMEOUT)) );
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

//------------------------------------------------------------------------------
// Green field B. LED pulse for green bit 7, that's it.

__attribute__((naked)) void green_field_B() {
	// set next callback
	asm("ldi r30, pm_lo8(blue_field_A)");
	asm("ldi r31, pm_hi8(blue_field_A)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");

	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(GREEN_FIELD_B_TIMEOUT)) );
	asm("ldi r31, %0" : : "M" (hi8(GREEN_FIELD_B_TIMEOUT)) );
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
// Blue field A. More LED pulses as above, the last of the audio processing, and
// button updating go here.

__attribute__((naked)) void blue_field_A() {
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
	asm("ldi r30, pm_lo8(blue_field_B)");
	asm("ldi r31, pm_hi8(blue_field_B)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");
	
	// set next timeout
	asm("ldi r30, %0" : : "M" (lo8(BLUE_FIELD_A_TIMEOUT)) );
	asm("ldi r31, %0" : : "M" (hi8(BLUE_FIELD_A_TIMEOUT)) );
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

	// restore r25 out here because the 80 cycle block is completely packed.
	asm("pop r25");

	// Restore status register, R31, and R30.
	asm("pop r30");
	asm("out 0x3f, r30");
	asm("pop r31");
	asm("pop r30");

	// Done
	asm("reti");
}	

//------------------------------------------------------------------------------
// Blue field B. Note that this callback is also responsible for triggering the
// next ADC sample conversion, so that it will happen during the relatively
// 'quiet' period between PWM periods. We also set the 'vblank' flag here so
// that clients that care about synchronizing with the PWM frequency can do so.

__attribute__((naked)) void blue_field_B() {
	// set next callback
	asm("ldi r30, pm_lo8(red_field_A)");
	asm("ldi r31, pm_hi8(red_field_A)");
	asm("sts timer_callback, r30");
	asm("sts timer_callback+1, r31");

	// set next timeout
	// 6 cycles
	asm("ldi r30, %0" : : "M" (lo8(BLUE_FIELD_B_TIMEOUT)) );
	asm("ldi r31, %0" : : "M" (hi8(BLUE_FIELD_B_TIMEOUT)) );
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
// Update button state. 27 cycles + call overhead. Uses r25, r30, r31
// This is separate from the LED interrupt handlers as we also need to be able
// to call it from our sleep mode watchdog interrupt.

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
  
  // The constants on the right here map from logical LED order to physical
  // order. They should probably be moved to config.h

	asm("ldd r18, z+%0*3" : : "X"(PIN_0_TO_PIXEL));
	asm("ldd r19, z+%0*3" : : "X"(PIN_1_TO_PIXEL));
	asm("ldd r20, z+%0*3" : : "X"(PIN_2_TO_PIXEL));
	asm("ldd r21, z+%0*3" : : "X"(PIN_3_TO_PIXEL));

	asm("ldd r22, z+%0*3" : : "X"(PIN_4_TO_PIXEL));
	asm("ldd r23, z+%0*3" : : "X"(PIN_5_TO_PIXEL));
	asm("ldd r24, z+%0*3" : : "X"(PIN_6_TO_PIXEL));
	asm("ldd r25, z+%0*3" : : "X"(PIN_7_TO_PIXEL));
	
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

void blip_swap() {
	while(blank);
	while(!blank);
	
	swap4c(&pixels[0].r, bits_RF);
	swap4c(&pixels[0].g, bits_GF);
	swap4c(&pixels[0].b, bits_BF);
}

void blip_swap64() {
  while (blip_tick & 63) {};
  blip_swap();
}  

__attribute__((naked)) void blip_clear() {
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

//---------------------------------------------------------------------------
// Minimalist LED test, just verifies that each LED can be addressed
// independently.

uint8_t extern const PROGMEM sources[];

void TestLEDs() {
	// Turn off the serial interface, which the bootloader leaves on by default.
	UCSR0B &= ~(1 << RXEN0);
	UCSR0B &= ~(1 << TXEN0);
	
	DDRB = 0xFF;
	PORTB = 0x00;
	
	DDRD = 0xFF;
	PORTD = 0x01;
	
	while(1)
	{
    PORTB = SINK_RED;
    for(int i = 0; i < 8; i++) {
		  PORTD = pgm_read_byte(sources + i);
      _delay_ms(300);
    }
    
    PORTB = SINK_GREEN;
    for(int i = 0; i < 8; i++) {
		  PORTD = pgm_read_byte(sources + i);
      _delay_ms(300);
    }

    PORTB = SINK_BLUE;
    for(int i = 0; i < 8; i++) {
		  PORTD = pgm_read_byte(sources + i);
      _delay_ms(300);
    }
	}	
}

//------------------------------------------------------------------------------
// Turns off every peripheral in the ATMega.

void ShutStuffDown() {
	// Set all ports to input mode.
	
	DDRB = 0x00;
	DDRC = 0x00;
	DDRD = 0x00;
	
	// Disable pullups on all ports except for our two button pins so we can come
  // out of sleep mode
	PORTB = 0x00;
	PORTC = (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN);
	PORTD = 0x00;
	
	// Disable analog comparator
	ACSR = bit(ACD);

  // TODO(aappleby): er, we should probably be turning off the digital input
  // buffer for the analog pins here, right?
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

//------------------------------------------------------------------------------
// Initial configuration of all peripherals - we set up the LED array, turn on
// the microphone, enable pull-ups for our buttons, start the ADC, configure our
// timer interrupts, and kick off the first ADC sample.

void SetupLEDs() {
  // No firing interrupts while we're configuring things.
	cli();
	
  // Turn absolutely everything off to start with.
	ShutStuffDown();
	
	// Port D is our LED source port.	
	PORTD = 0x00;
	DDRD = 0xFF;

	// Port B is our LED sink port.
	PORTB = 0x00;
	DDRB = 0xFF;

	// Port C is our button input, mic input, and status port. We also power the
  // microphone from this port so that we can turn it off during sleep mode.
	DDRC = (1 << MIC_POWER);
	PORTC = (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << MIC_POWER);

	// Turn the ADC on and set it up to read from the microphone input,
  // left-adjust the result, and sample in (14 * 32) = 448 cycles (56 uS
  // at 8 mhz) - the fastest we can sample and still get 10-bit resolution.

	PRR &= ~bit(PRADC);
	ADMUX  = MIC_PIN | bit(ADLAR);
	ADCSRA = bit(ADEN) | bit(ADPS2) | bit(ADPS0);
	DIDR0 = 0x3F & ~((1 << BUTTON1_PIN) | (1 << BUTTON2_PIN));
  DIDR1 = 0xFF;

	// Set timer 1 to tick at full speed and generate overflow interrupts.
	PRR &= ~bit(PRTIM1);
	TIMSK1 = (1 << TOIE1);
	TCCR1A = 0;
	TCCR1B = (1 << CS10);

  // The first callback that our timer interrupt will fire is for the first
  // half of the red field.
	timer_callback = red_field_A;
		
	// Device configured, kick off the first ADC conversion and enable
	// interrupts.
	sbi(ADCSRA,ADSC);

	sei();
}

//------------------------------------------------------------------------------
