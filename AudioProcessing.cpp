#include "Config.h"

//-----------------------------------------------------------------------------

#define TRIG1_STEP   1
#define TRIG1_CLAMP  60
#define BRIGHT1_UP   (65535 / 30)
#define BRIGHT1_DOWN (65535 / 300)
#define BRIGHT1_MAX  64

#define TRIG2_STEP   1
#define TRIG2_CLAMP  60
#define BRIGHT2_UP   (65535 / 40)
#define BRIGHT2_DOWN (65535 / 700)
#define BRIGHT2_MAX  64

#define DCFILTER    5
#define BASSFILTER  3

//-----------------------------------------------------------------------------

int16_t  tmax1;
int16_t  tmax2;

int16_t sample;
int16_t bass;

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

//----------
// Every 64 samples, adapt to volume
__attribute__((noinline)) void Adapt() {
	tickcount += 4;

	if(tickcount == 0)
	{
		if(brightaccum1 > (BRIGHT1_MAX*64))
		{
			if(tmax1 < 32000)
			{
				tmax1 += TRIG1_STEP;
				tmax1 += tmax1 >> 10;
			}
		}
		else
		{
			if(tmax1 > TRIG1_CLAMP)
			{
				tmax1 -= tmax1 >> 10;
				tmax1 -= TRIG1_STEP;
			}
		}

		if(brightaccum2 > (BRIGHT2_MAX*64))
		{
			if(tmax2 < 32000)
			{
				tmax2 += TRIG2_STEP;
				tmax2 += tmax2 >> 10;
			}
		}
		else
		{
			if(tmax2 > TRIG2_CLAMP)
			{
				tmax2 -= tmax2 >> 10;
				tmax2 -= TRIG2_STEP;
			}
		}

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
	}
	else
	{
		if(ibright2 >= BRIGHT2_DOWN)
		{
			ibright2 -= BRIGHT2_DOWN;
		}
	}
}

__attribute__((noinline)) void UpdateTreble() {
	if(sample > tmax1)
	{
		if(ibright1 <= (65535-BRIGHT1_UP))
		{
			ibright1 += BRIGHT1_UP;
		}
	}
	else
	{
		if(ibright1 >= BRIGHT1_DOWN)
		{
			ibright1 -= BRIGHT1_DOWN;
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
	Adapt();
	UpdateBass();
	UpdateTreble();
	ScaleValues();
	UpdateAccum();
}	