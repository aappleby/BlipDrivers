#include "AudioProcessing.h"

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "Defines.h"

//-----------------------------------------------------------------------------

uint16_t tmax1;
uint16_t tmax2;

uint16_t sample;
uint16_t bass;

int16_t accumD;
int16_t accumB;

uint16_t ibright1;
uint16_t ibright2;

uint16_t brightaccum1 = 0;
uint16_t brightaccum2 = 0;

uint8_t bright1 = 0;
uint8_t bright2 = 0;

uint8_t tickcount = 0;

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

//-----------------------------------------------------------------------------

/*
#define DCFILTER    5
__attribute__((noinline)) void RemoveRumble() {
	// remove rumble + residual DC bias
	sample -= accumD;
	accumD += (sample >> DCFILTER);
}
*/

/*
__attribute__((noinline)) void RemoveNoise() {
	// de-noise sample
	accumN = (accumN + sample) >> 1;
	sample = accumN;
}
*/

// split into bass & treble
/*
#define BASSFILTER  3
__attribute__((noinline)) void SplitBass() {
	sample -= accumB;
	accumB += (sample >> BASSFILTER);
}
*/

/*
__attribute__((noinline)) void NormalizeBass() {
	bass = (accumB < 0) ? -accumB : accumB;
}
*/

/*
__attribute__((noinline)) void NormalizeTreb() {
	if(sample < 0) sample = -sample;
}
*/


/*
__attribute__((noinline)) void Adapt1() {
	if(tickcount == 0) {
		uint16_t up = tmax1 >> 10;
		if(brightaccum1 >> 12)
		{
			tmax1 += 1;
			tmax1 += up;
		}
		else
		{
			tmax1 -= up;
			tmax1 -= 1;
		}
	}		
}
*/

/*
__attribute__((noinline)) void Adapt2() {
	if(tickcount == 0) {
		uint16_t up = tmax2 >> 10;
		if(brightaccum2 >> 12)
		{
			tmax2 += 1;
			tmax2 += up;
		}
		else
		{
			tmax2 -= up;
			tmax2 -= 1;
		}
	}		
}
*/

//----------
// Every 64 samples, adapt to volume

/*
__attribute__((noinline)) void Adapt3() {
	if(tickcount == 0) { brightaccum1 = 0; brightaccum2 = 0; }
}
*/

//----------
// Ramp our brightness up if we hit a trigger, down otherwise

/*
__attribute__((noinline)) void UpdateAudio() {
	uint16_t temp = ibright1;
	if(sample >= tmax1)
		temp = (temp <= (65535 - BRIGHT1_UP)) ? temp + BRIGHT1_UP : 65535;
	else
		temp = (temp >= BRIGHT1_DOWN) ? temp - BRIGHT1_DOWN : 0;
	ibright1 = temp;
	bright1 = pgm_read_byte(exptab+(ibright1 >> 8));
	brightaccum1 += pgm_read_byte(gammatab+bright1);

	temp = ibright2;
	if(bass >= tmax2)
		temp = (temp <= (65535 - BRIGHT2_UP)) ? temp + BRIGHT2_UP : 65535;
	else
		temp = (temp >= BRIGHT2_DOWN) ? temp - BRIGHT2_DOWN : 0;
	ibright2 = temp;
	bright2 = pgm_read_byte(exptab+(ibright2 >> 8));
	brightaccum2 += pgm_read_byte(gammatab+bright2);
}
*/

__attribute__((naked)) void UpdateAudioSync() {
	asm("push r28");
	asm("push r29");
	asm("push r30");
	asm("push r31");
	
	/*
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
	*/		
	
	/*
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
	*/

	/*
	// 4 cycles
	{
		// ibright1 = temp;
		asm("store_bright1:");
		asm("sts ibright1 + 0, r28");
		asm("sts ibright1 + 1, r29");
	}
	*/		
	
	/*
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
	*/
	
	/*
	// 17 cycles
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
		asm("adc r31, r29");
		asm("sts brightaccum1 + 0, r30");
		asm("sts brightaccum1 + 1, r31");
	}
	*/		
	
	/*
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
	*/		
	
	/*
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
	*/

	/*
	// 4 cycles
	{
		// ibright2 = temp;
		asm("store_bright2:");
		asm("sts ibright2 + 0, r28");
		asm("sts ibright2 + 1, r29");
	}
	*/
	
	/*
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
	*/		

	/*
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
	*/		
	
	asm("pop r31");
	asm("pop r30");
	asm("pop r29");
	asm("pop r28");
	asm("ret");
}

//-----------------------------------------------------------------------------
	