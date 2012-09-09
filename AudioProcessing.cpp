#include "Config.h"

//-----------------------------------------------------------------------------

#define TRIG1_CLAMP  60
#define BRIGHT1_UP   (65535 / 30)
#define BRIGHT1_DOWN (65535 / 300)

#define TRIG2_CLAMP  60
#define BRIGHT2_UP   (65535 / 40)
#define BRIGHT2_DOWN (65535 / 700)

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

//----------
// Every 64 samples, adapt to volume

__attribute__((noinline)) void Adapt3() {
	if(tickcount == 0)
	{
		brightaccum1 = 0;
		brightaccum2 = 0;
	}
}	

//----------
// Ramp our brightness up if we hit a trigger, down otherwise
__attribute__((noinline)) void UpdateBass() {
	if(bass > tmax2)
	{
		if(ibright2 <= (65535-BRIGHT2_UP))
		{
			ibright2 += BRIGHT2_UP;
		}
		else {
			ibright2 = 65535;
		}			
	}
	else
	{
		if(ibright2 & 0xFF00)
		{
			ibright2 -= BRIGHT2_DOWN;
		} else {
			ibright2 = 0;
		}			
	}
}

__attribute__((noinline)) void UpdateTreble() {
	if(sample > tmax1)
	{
		if(ibright1 < (65535-BRIGHT1_UP))
		{
			ibright1 += BRIGHT1_UP;
		}
		else {
			ibright1 = 65535;
		}			
	}
	else
	{
		if(ibright1 > BRIGHT1_DOWN)
		{
			ibright1 -= BRIGHT1_DOWN;
		}
		else {
			ibright1 = 0;
		}			
	}
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
	Clamp2();
	Adapt1();
	Adapt2();
	Adapt3();
	UpdateBass();
	UpdateTreble();
	ScaleValues();
	UpdateAccum();
}	