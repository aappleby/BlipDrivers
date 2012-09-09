#include "Config.h"

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
__attribute__((naked)) void Clamp2() {
	// if(tmax2 & 0x8000) tmax2 -= 256;
	// if(tmax2 < TRIG2_CLAMP) tmax2++;
	// 17 cycles

	asm("lds r30, tmax2 + 0");
	asm("lds r31, tmax2 + 1");

	// Clamp if above 32767
	asm("sbrc r31, 7");
	asm("subi r31, 0x01");
	
	// Clamp if below 60
	asm("cpi r30, 60");
	asm("cpc r31, r1"); // THIS NEEDS A ZERO REGISTER
	asm("brge trig2_noclamp");
	asm("adiw r30, 1");
	asm("rjmp trig2_clampdone");
	
	asm("trig2_noclamp:");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("trig2_clampdone:");
	
	asm("sts tmax2 + 0, r30");
	asm("sts tmax2 + 1, r31");
	asm("ret");
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

// if(tickcount == 0) { brightaccum1 = 0; brightaccum2 = 0; }
// 14 cycles
/*
__attribute__((naked)) void Adapt3() {
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
	asm("ret");
}	
*/
	
//----------
// Ramp our brightness up if we hit a trigger, down otherwise

/*
__attribute__((noinline)) void UpdateBass() {
	uint16_t temp = ibright2;
	if(bass >= tmax2)
	{
		temp = (temp <= (65535 - BRIGHT2_UP)) ? temp + BRIGHT2_UP : 65535;
	}
	else
	{
		temp = (temp >= BRIGHT2_DOWN) ? temp - BRIGHT2_DOWN : 0;
	}
	ibright2 = temp;
}
*/

__attribute__((naked)) void UpdateBass() {
	asm("push r28");
	asm("push r29");
	asm("push r30");
	asm("push r31");
	
	asm("lds r28, bass + 0");
	asm("lds r29, bass + 1");
	asm("lds r30, tmax2 + 0");
	asm("lds r31, tmax2 + 1");
	asm("cp r28, r30");
	asm("cpc r29, r31");
	asm("brcs signal_low2");
	asm("nop");
	
	asm("signal_high2:");
	asm("lds r28, ibright2 + 0");
	asm("lds r29, ibright2 + 1");
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
	asm("lds r28, ibright2 + 0");
	asm("lds r29, ibright2 + 1");
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

	asm("store_bright2:");
	asm("sts ibright2 + 0, r28");
	asm("sts ibright2 + 1, r29");
	
	asm("pop r31");
	asm("pop r30");
	asm("pop r29");
	asm("pop r28");
	asm("ret");
}

/*
__attribute__((noinline)) void UpdateTreble() {
	uint16_t temp = ibright1;
	if(sample >= tmax1)
	{
		temp = (temp <= (65535 - BRIGHT1_UP)) ? temp + BRIGHT1_UP : 65535;
	}
	else
	{
		temp = (temp >= BRIGHT1_DOWN) ? temp - BRIGHT1_DOWN : 0;
	}
	ibright1 = temp;
}
*/

__attribute__((naked)) void UpdateTreble() {
	asm("push r28");
	asm("push r29");
	asm("push r30");
	asm("push r31");
	
	asm("lds r28, sample + 0");
	asm("lds r29, sample + 1");
	asm("lds r30, tmax1 + 0");
	asm("lds r31, tmax1 + 1");
	asm("cp r28, r30");
	asm("cpc r29, r31");
	asm("brcs signal_low1");
	asm("nop");
	
	asm("signal_high1:");
	asm("lds r28, ibright1 + 0");
	asm("lds r29, ibright1 + 1");
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
	asm("lds r28, ibright1 + 0");
	asm("lds r29, ibright1 + 1");
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

	asm("store_bright1:");
	asm("sts ibright1 + 0, r28");
	asm("sts ibright1 + 1, r29");
	
	asm("pop r31");
	asm("pop r30");
	asm("pop r29");
	asm("pop r28");
	asm("ret");
}


__attribute__((noinline)) void ScaleValues() {
	bright1 = pgm_read_byte(exptab+(ibright1 >> 8));
	bright2 = pgm_read_byte(exptab+(ibright2 >> 8));
}	

__attribute__((noinline)) void UpdateAccum()
{
	brightaccum1 += pgm_read_byte(gammatab+bright1);
	brightaccum2 += pgm_read_byte(gammatab+bright2);
}

//-----------------------------------------------------------------------------

__attribute__((noinline)) void UpdateAudioSync() {
	//SplitBass();
	//NormalizeBass();
	//NormalizeTreb();
	//Clamp1();
	//Clamp2();
	//Adapt1();
	//Adapt2();
	//Adapt3();
	UpdateBass();
	UpdateTreble();
	ScaleValues();
	UpdateAccum();
}	