#include "Patterns.h"

#include "LEDDriver.h"
#include "Tables.h"

void Gradients() {
	r[0] = (224 * bright2) / 256;
	r[1] = (192 * bright2) / 256;
	r[2] = (160 * bright2) / 256;
	r[3] = (128 * bright2) / 256;
	r[4] = (96 * bright2) / 256;
	r[5] = (64 * bright2) / 256;
	r[6] = (32 * bright2) / 256;
	r[7] = (0 * bright2) / 256;
		
	g[0] = (32  * bright1) / 256;
	g[1] = (64  * bright1) / 256;
	g[2] = (128 * bright1) / 256;
	g[3] = (255 * bright1) / 256;
	g[4] = (255 * bright1) / 256;
	g[5] = (128 * bright1) / 256;
	g[6] = (64  * bright1) / 256;
	g[7] = (32  * bright1) / 256;

	b[0] = (0 * bright2) / 256;
	b[1] = (32 * bright2) / 256;
	b[2] = (64 * bright2) / 256;
	b[3] = (96 * bright2) / 256;
	b[4] = (128 * bright2) / 256;
	b[5] = (160 * bright2) / 256;
	b[6] = (192 * bright2) / 256;
	b[7] = (224 * bright2) / 256;
}

void RGBWaves() {
	static uint16_t timerR;
	static uint16_t timerG;
	static uint16_t timerB;
	
	const int stepR = 14;
	const int stepG = 32;
	const int stepB = 45;
	const int speedR = 25;
	const int speedG = -13;
	const int speedB = 17;
	
	timerR += speedR;
	timerG += speedG;
	timerB += speedB;
	uint8_t phaseR = timerR >> 8;
	uint8_t phaseG = timerG >> 8;
	uint8_t phaseB = timerB >> 8;
	
	uint8_t t, v;
	for(uint8_t i = 0; i < 8; i++) {
		t = phaseR + stepR * i;
		r[i] = (getGammaSin(t) * bright2) >> 8;
		
		t = phaseG + stepG * i;
		g[i] = (getGammaSin(t) * bright1) >> 8;

		t = phaseB + stepB * i;
		b[i] = (getGammaSin(t) * bright2) >> 8;
	}		
}	
 

void StartupPattern() {
	static uint16_t timerR;
	static uint16_t timerG;
	static uint16_t timerB;
	
	/*
	const int stepR = 14;
	const int stepG = 32;
	const int stepB = 45;
	const int speedR = 25;
	const int speedG = -13;
	const int speedB = 17;
	*/
	const int stepR = 25;
	const int stepG = 26;
	const int stepB = 27;
	const int speedR = -4;
	const int speedG = -3;
	const int speedB = -7;
	
	
	timerR += speedR;
	timerG += speedG;
	timerB += speedB;
	uint8_t phaseR = timerR >> 8;
	uint8_t phaseG = timerG >> 8;
	uint8_t phaseB = timerB >> 8;

	r[0] = phaseR + stepR * 0;
	r[1] = phaseR + stepR * 1;
	r[2] = phaseR + stepR * 2;
	r[3] = phaseR + stepR * 3;
	r[4] = phaseR + stepR * 4;
	r[5] = phaseR + stepR * 5;
	r[6] = phaseR + stepR * 6;
	r[7] = phaseR + stepR * 7;

	r[0] = getGammaSin(r[0]);
	r[1] = getGammaSin(r[1]);
	r[2] = getGammaSin(r[2]);
	r[3] = getGammaSin(r[3]);
	r[4] = getGammaSin(r[4]);
	r[5] = getGammaSin(r[5]);
	r[6] = getGammaSin(r[6]);
	r[7] = getGammaSin(r[7]);

	g[0] = phaseG + stepG * 0;
	g[1] = phaseG + stepG * 1;
	g[2] = phaseG + stepG * 2;
	g[3] = phaseG + stepG * 3;
	g[4] = phaseG + stepG * 4;
	g[5] = phaseG + stepG * 5;
	g[6] = phaseG + stepG * 6;
	g[7] = phaseG + stepG * 7;

	g[0] = getGammaSin(g[0]);
	g[1] = getGammaSin(g[1]);
	g[2] = getGammaSin(g[2]);
	g[3] = getGammaSin(g[3]);
	g[4] = getGammaSin(g[4]);
	g[5] = getGammaSin(g[5]);
	g[6] = getGammaSin(g[6]);
	g[7] = getGammaSin(g[7]);
	
	b[0] = phaseB + stepB * 0;
	b[1] = phaseB + stepB * 1;
	b[2] = phaseB + stepB * 2;
	b[3] = phaseB + stepB * 3;
	b[4] = phaseB + stepB * 4;
	b[5] = phaseB + stepB * 5;
	b[6] = phaseB + stepB * 6;
	b[7] = phaseB + stepB * 7;

	b[0] = getGammaSin(b[0]);
	b[1] = getGammaSin(b[1]);
	b[2] = getGammaSin(b[2]);
	b[3] = getGammaSin(b[3]);
	b[4] = getGammaSin(b[4]);
	b[5] = getGammaSin(b[5]);
	b[6] = getGammaSin(b[6]);
	b[7] = getGammaSin(b[7]);
	
	/*	
	for(int i = 0; i < 8; i++) {
		r[i] = (r[i] * bright1) / 256;
		g[i] = (g[i] * bright2) / 256;
		b[i] = (b[i] * bright1) / 256;
	}
	*/
}	

void DumbPattern() {
	r[0] = bright1;
	r[1] = bright1;
	r[2] = bright1;
	r[3] = bright1;
	
	g[4] = bright2;
	g[5] = bright2;
	g[6] = bright2;
	g[7] = bright2;
}

void Sparkles() {
	static uint16_t x[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
	x[0] += 9;
	x[1] += 12;
	x[2] += 7;
	x[3] += 13;
	x[4] += 8;
	x[5] += 11;
	x[6] += 10;
	x[7] += 9;
	
	b[0] = getSparkle(uint8_t(x[0] >> 8));
	b[1] = getSparkle(uint8_t((x[1] >> 8) + 118));
	b[2] = getSparkle(uint8_t((x[2] >> 8) + 22));
	b[3] = getSparkle(uint8_t((x[3] >> 8) + 200));
	b[4] = getSparkle(uint8_t((x[4] >> 8) + 230));
	b[5] = getSparkle(uint8_t((x[5] >> 8) + 92));
	b[6] = getSparkle(uint8_t((x[6] >> 8) + 60));
	b[7] = getSparkle(uint8_t((x[7] >> 8) + 157));
	
	for(int i = 0; i < 8; i++) b[i] = (b[i] * bright1) >> 8;
	
	r[0] = g[0] = 5;
	r[1] = g[1] = 48;
	r[2] = g[2] = 128;
	r[3] = g[3] = 255;
	r[4] = g[4] = 255;
	r[5] = g[5] = 128;
	r[6] = g[6] = 48;
	r[7] = g[7] = 5;
	
	for(int i = 0; i < 8; i++) {
		r[i] = (r[i] * bright2) >> 8;
		g[i] = (g[i] * bright2) >> 8;
	}		
}

void Scroller() {
	static uint8_t buffer[256];
	static uint8_t tick = 0;
	static uint8_t cursor = 0;
	
	tick++;
	if(tick == 7) {
		tick = 0;
		cursor--;
		buffer[cursor] = (bright2 * bright2) >> 8;
	}
	
	for(int i = 0; i < 8; i++) {
		uint8_t x1 = (i * 16) + cursor;
		g[i] = buffer[x1];
		uint8_t x2 = (i * 32) + cursor;
		r[i] = buffer[x2];
		uint8_t x3 = (i * 24) + cursor;
		b[i] = buffer[x3];
	}
}

const uint8_t pixel16[256] PROGMEM =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255,

	255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void SpaceZoom() {
	static uint8_t tick1 = 0;
	static uint8_t tick2 = 0;
	static uint8_t frame = 0;
	static uint8_t cursor = 0;
	static uint8_t buffer[256];
	
	tick1++;
	if(tick1 == 11)
	{
		tick1 = 0;
		cursor--;
		buffer[cursor] = (bright2 * bright2) >> 8;
	}
	
	tick2++;
	if(tick2 == 100) {
		frame++;
		tick2 = 0;
	}
	
	//for(int i = 0; i < 8; i++) { r[i] = g[i] = b[i] = bright2;}
	//return;
	
	for(int i = 0; i < 8; i++) {
		uint8_t x = (i * 16 - 63) + 128;
		uint8_t s = getSin(frame);
		uint8_t shift = s/4 - (256 / 8);
		
		x += shift;
		x += 128;
		
		uint8_t t = (255 - getQuad(x));

		uint8_t t1 = t / 4 + cursor;
		uint8_t t2 = t / 5 + cursor;
		uint8_t t3 = t / 3 + cursor;
		
		r[i] = buffer[t1];
		g[i] = buffer[t2];
		b[i] = buffer[t3];
	}
}