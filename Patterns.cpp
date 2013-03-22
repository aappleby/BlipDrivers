#include "Patterns.h"
#include "Tables.h"
#include "LEDDriver.h"

#include <avr/pgmspace.h>
#include <math.h>


__attribute__((noinline)) uint8_t rng()
{
	static uint32_t y = 12345678;
	
	y ^= y << 1;
	y ^= y >> 5;
	y ^= y << 16;
	
	return y;
}

void button_test() {
	clear();
	for(int i = 0; i < 4; i++) {
		if(debounce_down1) pixels[i].g = 0xFF;
		else pixels[i].r = 0xFF;
	}
	for(int i = 4; i < 8; i++) {
		if(debounce_down2) pixels[i].g = 0xFF;
		else pixels[i].r = 0xFF;
	}
}

void red_test() {
	uint8_t phase = blip_tick / 16;
	const int step = 35;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].r = pgm_read_byte(gammasin + t);
	}
}	

void green_test() {
	uint8_t phase = blip_tick / 16;
	const int step = 35;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].g = pgm_read_byte(gammasin + t);
	}
}

void blue_test() {
	uint8_t phase = blip_tick / 16;
	const int step = 35;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].b = pgm_read_byte(gammasin + t);
	}
}

void cie_test() {
	uint8_t phase = blip_tick / 256;
	const int step = 0;

	for(int i = 0; i < 4; i++) {
		uint8_t t = phase + step * i;
		uint8_t c = (t * t) >> 8;
		//uint8_t c = pgm_read_byte(gammasin + t);
		pixels[i].g = c;
	}

	for(int i = 4; i < 8; i++) {
		uint8_t t = phase + step * i;
		//uint8_t c = pgm_read_byte(ciesin + t);
		uint8_t c = pgm_read_byte(cielum + t);
    //uint8_t c = cielum[t];
		pixels[i].g = c;
	}
}

void pov_test() {
	uint8_t phaseG = (blip_tick / 16);
	uint8_t phaseB = (blip_tick / 16) + 12;
	uint8_t phaseR = (blip_tick / 16) + 24;
	const int step = 36;

	for(int i = 0; i < 8; i++) {
		uint8_t t;
		t = phaseG + step * i;
		pixels[i].g = pgm_read_byte(gammasin + t);
		t = phaseB + step * i;
		pixels[i].b = pgm_read_byte(gammasin + t);
		t = phaseR + step * i;
		pixels[i].r = pgm_read_byte(gammasin + t);
	}
}

void pov_test2() {
	static uint8_t delay = 0;
	static uint8_t phase = 0;
	static uint8_t color = 0;
	static uint8_t pixel = 0;
	
	delay++;
	if(delay == 9)
	{
		delay = 0;
		phase++;
	}
	if(phase == 255)
	{
		phase = 0;
		color++;
	}
	if(color == 3)
	{
		color = 0;
		pixel++;
	}
	if(pixel == 8)
	{
		pixel = 0;
	}		

	clear();

	uint8_t bright = pgm_read_byte(gammasin + uint8_t(phase));
	
	switch(color) 
	{
	case 0: { pixels[pixel].g = bright; return; }
	case 1: { pixels[pixel].b = bright; return; }
	case 2: { pixels[pixel].r = bright; return; }
	case 3: { return; }
	}
}


void audio_test() {
	pixels[0].r = pixels[0].g = pixels[0].b = bright1;
	pixels[1].r = pixels[1].g = pixels[1].b = bright1;
	pixels[2].r = pixels[2].g = pixels[2].b = bright2;
	pixels[3].r = pixels[3].g = pixels[3].b = bright2;
	pixels[4].r = pixels[4].g = pixels[4].b = bright2;
	pixels[5].r = pixels[5].g = pixels[5].b = bright2;
	pixels[6].r = pixels[6].g = pixels[6].b = bright1;
	pixels[7].r = pixels[7].g = pixels[7].b = bright1;
}	

void PulseFade() {
	static uint16_t phases[8];
	
	for(uint8_t i = 0; i < 8; i++) {
		phases[i] += rng() & 3;
		uint8_t cursor = phases[i] >> 4;
		pixels[i].r = pgm_read_byte(pulse_2_2 + cursor);
		pixels[i].g = pgm_read_byte(pulse_2_3 + cursor);
		pixels[i].b = pgm_read_byte(pulse_2_6 + cursor);
	}
}

void Speed() {
	static uint8_t dir = 0;
	static uint8_t mode = 0;
	
	uint8_t beat = 0;
	
	if(mode == 0) {
		if(ibright2 > 32768 + 20000) {
			mode = 1;
			beat = 1;
		}			
	} else {
		if(ibright2 < 32768 - 20000) {
			mode = 0;
		}			
	}		

	// 24-bit counter
	static uint32_t phase = 0;
	static uint32_t speed = 6000;

	phase += speed;

	static uint16_t timer = 0;
	timer++;

	static uint8_t miss = 0;
	
	if(timer > 4096)
	{
		miss = (miss + 1) & 0x7f;
	}
	

	if(beat)
	{
		if(timer < 1536)
		{
			// Early beat, ignore.
			//pixels[5].g = 255;
			beat = false;
		}
		else if (timer > 4096)
		{
			// Missed beat, resync but don't adjust.
			//pixels[6].g = 255;
			beat = false;
			timer = 0;
		}
		else 
		{
			// Hit the beat.
			pixels[7].g = 255;
			timer = 0;
			
			uint16_t phase2 = (phase >> 16);
			
			if(miss == 0)
			{
				if(phase2 > 260)
				{
					pixels[4].g = 255;
					speed -= 100;
				}
				else if(phase2 < 250)
				{
					pixels[5].g = 255;
					speed += 100;
				}
			}
			miss = 0;
			
			phase = 0;
			dir = !dir;
		}
	}
	
	if(miss)
	{
		pixels[6].b = 0xff;		
	}
	
	if(dir)
	{
		pixels[0].r = 255;
	}
	else
	{
		pixels[1].r = 255;
	}
	
	uint8_t phase2 = (phase >> 16);
	
	pixels[2].g = bright2;
	pixels[3].g = ((phase2 < 32) || (phase2 > 220)) ? 0xFF : 0x00;
	
	/*
	if(dir) {
		timer += (ibright1 >> 10);
	} else {
		timer -= (ibright1 >> 10);
	}				
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].r = pixels[i].b = pgm_read_byte(gammasin + t);
	}
	*/
}


void Speed2() {
	static uint8_t dir = 0;
	static uint8_t mode = 0;
	static uint16_t timer;
	
	if(mode == 0) {
		if(ibright2 > 16384 + 8192) {
			mode = 1;
			dir = !dir;
		}
	} else {
		if(ibright2 < 8192) {
			mode = 0;
		}
	}
	
	const int step = 35;
	
	if(dir) {
		timer += (ibright1 >> 10);
	} else {
		timer -= (ibright1 >> 10);
	}
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t1 = phase + (step * i);
		uint8_t t2 = (255-phase) + (step * i);
		uint8_t a = pgm_read_byte(pulse_5_4 + t1);
		uint8_t b = pgm_read_byte(pulse_5_4 + t2);
		pixels[i].g = a;
		pixels[i].b = b;
	}
}


void Gradients() {
	pixels[0].r = (224 * bright2) / 256;
	pixels[1].r = (192 * bright2) / 256;
	pixels[2].r = (160 * bright2) / 256;
	pixels[3].r = (128 * bright2) / 256;
	pixels[4].r = (96 * bright2) / 256;
	pixels[5].r = (64 * bright2) / 256;
	pixels[6].r = (32 * bright2) / 256;
	pixels[7].r = (0 * bright2) / 256;
		
	pixels[0].g = (32  * bright2) / 256;
	pixels[1].g = (64  * bright2) / 256;
	pixels[2].g = (128 * bright2) / 256;
	pixels[3].g = (255 * bright2) / 256;
	pixels[4].g = (255 * bright2) / 256;
	pixels[5].g = (128 * bright2) / 256;
	pixels[6].g = (64  * bright2) / 256;
	pixels[7].g = (32  * bright2) / 256;

	pixels[0].b = (0 * bright2) / 256;
	pixels[1].b = (32 * bright2) / 256;
	pixels[2].b = (64 * bright2) / 256;
	pixels[3].b = (96 * bright2) / 256;
	pixels[4].b = (128 * bright2) / 256;
	pixels[5].b = (160 * bright2) / 256;
	pixels[6].b = (192 * bright2) / 256;
	pixels[7].b = (224 * bright2) / 256;
}

void RGBWaves() {
	const int stepR = 14;
	const int stepG = 32;
	const int stepB = 45;

	uint8_t phaseR = blip_tick / 10;
	uint8_t phaseG = -blip_tick / 19;
	uint8_t phaseB = blip_tick / 15;
	
	uint8_t t;
	for(uint8_t i = 0; i < 8; i++) {
		t = phaseR + stepR * i;
		pixels[i].r = (pgm_read_byte(gammasin + t) * bright2) >> 8;
		
		t = phaseG + stepG * i;
		pixels[i].g = (pgm_read_byte(gammasin + t) * bright1) >> 8;

		t = phaseB + stepB * i;
		pixels[i].b = (pgm_read_byte(gammasin + t) * bright2) >> 8;
	}		
}	
 

void StartupPattern() {
	const int stepR = 25;
	const int stepG = 26;
	const int stepB = 27;

	uint8_t phaseR = blip_tick / 79;
	uint8_t phaseG = blip_tick / 82;
	uint8_t phaseB = blip_tick / 65;
	
	uint8_t cursorR = phaseR;
	uint8_t cursorG = phaseG;
	uint8_t cursorB = phaseB;
	for(int i = 0; i < 8; i++) {
		pixels[i].r = pgm_read_byte(gammasin + cursorR);
		pixels[i].g = pgm_read_byte(gammasin + cursorG);
		pixels[i].b = pgm_read_byte(gammasin + cursorB);
		
		cursorR += stepR;
		cursorG += stepG;
		cursorB += stepB;
	}		
}	

void FastWaves() {
	static uint16_t timerR;
	static uint16_t timerG;
	static uint16_t timerB;
	
	const int stepR = 25;
	const int stepG = 36;
	const int stepB = 37;
	const int speedR = -327;
	const int speedG = -528;
	const int speedB = 629;
	
	timerR += speedR;// + (bright1 >> 2);
	timerG += speedG;
	timerB += speedB;
	uint8_t phaseR = timerR >> 8;
	uint8_t phaseG = timerG >> 8;
	uint8_t phaseB = timerB >> 8;
	
	uint8_t cursorR = phaseR;
	uint8_t cursorG = phaseG;
	uint8_t cursorB = phaseB;
	for(int i = 0; i < 8; i++) {
		pixels[i].r = pgm_read_byte(gammapulse + cursorR);
		pixels[i].g = pgm_read_byte(gammapulse + cursorG);
		pixels[i].b = pgm_read_byte(gammapulse + cursorB);
		
		cursorR += stepR;
		cursorG += stepG;
		cursorB += stepB;
	}
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
	
	pixels[0].b = pgm_read_byte(sparkles + uint8_t(x[0] >> 8));
	pixels[1].b = pgm_read_byte(sparkles + uint8_t((x[1] >> 8) + 118));
	pixels[2].b = pgm_read_byte(sparkles + uint8_t((x[2] >> 8) + 22));
	pixels[3].b = pgm_read_byte(sparkles + uint8_t((x[3] >> 8) + 200));
	pixels[4].b = pgm_read_byte(sparkles + uint8_t((x[4] >> 8) + 230));
	pixels[5].b = pgm_read_byte(sparkles + uint8_t((x[5] >> 8) + 92));
	pixels[6].b = pgm_read_byte(sparkles + uint8_t((x[6] >> 8) + 60));
	pixels[7].b = pgm_read_byte(sparkles + uint8_t((x[7] >> 8) + 157));
	
	for(int i = 0; i < 8; i++) {
		pixels[i].b = (pixels[i].b * bright1) >> 8;
	}		
	
	pixels[0].r = pixels[0].g = 5;
	pixels[1].r = pixels[1].g = 48;
	pixels[2].r = pixels[2].g = 128;
	pixels[3].r = pixels[3].g = 255;
	pixels[4].r = pixels[4].g = 255;
	pixels[5].r = pixels[5].g = 128;
	pixels[6].r = pixels[6].g = 48;
	pixels[7].r = pixels[7].g = 5;
	
	for(int i = 0; i < 8; i++) {
		pixels[i].r = (pixels[i].r * bright2) >> 8;
		pixels[i].g = (pixels[i].g * bright2) >> 8;
	}		
}

void Sparklefest() {
	static uint8_t cursors[8];
	
	if(blip_tick & 7) return;
	
	for(int i = 0; i < 8; i++) 
	{
		if(cursors[i] > 0) {
			cursors[i] += 1;
			if(cursors[i] > 250) cursors[i] = 0;
		}
		else 
		{
			uint8_t limit = (bright2 >> 5);
			if(rng() < limit) {
				cursors[i] = 1;
			}
		}
		
		pixels[i].g = pgm_read_byte(gammapulse + cursors[i]);
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
		pixels[i].g = buffer[x1];
		uint8_t x2 = (i * 32) + cursor;
		pixels[i].r = buffer[x2];
		uint8_t x3 = (i * 24) + cursor;
		pixels[i].b = buffer[x3];
	}
}

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
	
	for(int i = 0; i < 8; i++) {
		uint8_t x = (i * 16 - 63) + 128;
		uint8_t s = pgm_read_byte(sintab + frame);
		uint8_t shift = s/4 - (256 / 8);
		
		x += shift;
		x += 128;
		
		uint8_t t = (255 - pgm_read_byte(quadtab + x));

		uint8_t t1 = t / 4 + cursor;
		uint8_t t2 = t / 5 + cursor;
		uint8_t t3 = t / 3 + cursor;
		
		pixels[i].r = buffer[t1];
		pixels[i].g = buffer[t2];
		pixels[i].b = buffer[t3];
	}
}

/*
void float_test() {
	float t, c;
	t = blip_tick;
	t /= 4096.0 * 8;
	t *= 3.14159265 * 2.0;
	c = 0;
	for(int i = 0; i < 8; i++) {
		float x = sin(t + c);
		x = (x + 1) / 2;
		x = x*x;
		//x *= bright1;
		x *= 255;
		r[i] = x;
		c += (3.1415926535 * 2.0) / 8.0;
	}
		
	t = blip_tick;
	t /= 3123.0 * 8;
	t *= 3.14159265 * 2.0;
	c = 0;
	for(int i = 0; i < 8; i++) {
		float x = sin(t + c);
		x = (x + 1) / 2;
		x = x*x;
		//x *= bright1;
		x *= 255;
		g[i] = x;
		c += (3.1415926535 * 2.0) / 8.0;
	}

	t = blip_tick;
	t /= 2712.0 * 8;
	t *= 3.14159265 * 2.0;
	c = 0;
	for(int i = 0; i < 8; i++) {
		float x = sin(t + c);
		x = (x + 1) / 2;
		x = x*x;
		//x *= bright1;
		x *= 255;
		b[i] = x;
		c += (3.1415926535 * 2.0) / 8.0;
	}
}
*/

void VUMeter() {
	uint16_t 
	clear();
	uint16_t b = ibright2 >> 6;
	for(int i = 3; i >= 0; i--) {
		if(b > 256) {
			pixels[i].r = 0xFF;
			pixels[i].g = 0xFF >> 2;
			b -= 256;
		}
		else 
		{
			pixels[i].r = b;
			pixels[i].g = b >> 2;
			break;
		}
	}
	b = ibright1 >> 6;
	for(int i = 4; i < 8; i++) {
		if(b > 256) {
			pixels[i].b = 0xFF >> 2;
			pixels[i].g = 0xFF;
			b -= 256;
		}
		else
		{
			b = (b * b) >> 8;
			pixels[i].b = b >> 2;
			pixels[i].g = b;
			break;
		}
	}
}

void crosscross() {
	Pixel dark[8] = {
		{   0,   0,   0 },
		{   0,   0,   0 },
		{   0,   0,   0 },
		{   0,   0,   0 },
		{   0,   0,   0 },
		{   0,   0,   0 },
		{   0,   0,   0 },
		{   0,   0,   0 },
	};

	Pixel pulse[8] = {
		{   5,   0,   0 },
		{  48,   5,   0 },
		{ 128,  48,   0 },
		{ 255, 192,  32 },
		{ 255, 192,  32 },
		{ 128,  48,   0 },
		{  48,   5,   0 },
		{   5,   0,   0 },
	};

	Pixel ears[8] = {
		{   0, 128,   0 },
		{   0,  64,   0 },
		{   0,  32,   0 },
		{   0,  16,   0 },
		{   0,  16,   0 },
		{   0,  32,   0 },
		{   0,  64,   0 },
		{   0, 128,   0 },
	};

	static uint16_t timer;
	
	const int speed = 77;
	
	timer += speed;
	uint8_t phase = timer >> 8;
	for(int i = 0; i < 8; i++) {
		uint8_t r1 = ((dark[i].r * (255-bright2)) + (pulse[i].r * bright2)) >> 8;
		uint8_t g1 = ((dark[i].g * (255-bright2)) + (pulse[i].g * bright2)) >> 8;
		uint8_t b1 = ((dark[i].b * (255-bright2)) + (pulse[i].b * bright2)) >> 8;

		uint8_t r2 = (ears[i].r * bright1) >> 8;
		uint8_t g2 = (ears[i].g * bright1) >> 8;
		uint8_t b2 = (ears[i].b * bright1) >> 8;
		
		//uint8_t s = pgm_read_byte(gammasin + cursor);
		//cursor += step;
		pixels[i].r = (r1 + r2);
		pixels[i].g = (g1 + g2);
		pixels[i].b = (b1 + b2 + 8);
	}		
}	

/*
void blah() {
	FastWaves();

	for(int i = 0; i < 8; i++) {
		pixels[i].r = (pixels[i].r * bright2) >> 8;
		pixels[i].g = (pixels[i].g * bright2) >> 8;
		pixels[i].b = (pixels[i].b * bright2) >> 8;
	}		
}
*/



