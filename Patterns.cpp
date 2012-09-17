#include "Patterns.h"

#include "LEDDriver.h"

#include <avr/pgmspace.h>
#include <math.h>

//-----------------------------------------------------------------------------

const uint8_t sintab[256] PROGMEM =
{	
128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173,
176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 
245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 
255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 
245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 
218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 
176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 
128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 
76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 
27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 
1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 
18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 
65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124
};

const uint8_t quadtab[256] PROGMEM =
{
255, 251, 247, 243, 239, 235, 232, 228, 224, 220, 217, 213, 209, 206, 202, 199, 195, 192, 
188, 185, 181, 178, 175, 171, 168, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 
131, 128, 126, 123, 120, 117, 115, 112, 109, 107, 104, 102, 99, 97, 94, 92, 89, 87, 85, 82, 
80, 78, 76, 74, 71, 69, 67, 65, 63, 61, 59, 57, 56, 54, 52, 50, 48, 47, 45, 43, 42, 40, 38, 
37, 35, 34, 32, 31, 30, 28, 27, 26, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 
10, 9, 9, 8, 7, 7, 6, 5, 5, 4, 4, 3, 3, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 7, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 
17, 18, 19, 20, 21, 22, 23, 24, 26, 27, 28, 30, 31, 32, 34, 35, 37, 38, 40, 42, 43, 45, 47, 
48, 50, 52, 54, 56, 57, 59, 61, 63, 65, 67, 69, 71, 74, 76, 78, 80, 82, 85, 87, 89, 92, 94, 
97, 99, 102, 104, 107, 109, 112, 115, 117, 120, 123, 126, 128, 131, 134, 137, 140, 143, 146, 
149, 152, 155, 158, 162, 165, 168, 171, 175, 178, 181, 185, 188, 192, 195, 199, 202, 206, 209, 
213, 217, 220, 224, 228, 232, 235, 239, 243, 247, 251, 255,
};

// Gamma-corrected sine wave
const uint8_t gammasin[] PROGMEM =
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

const uint8_t sparkles[256] PROGMEM =
{
	// Exponential spike, scale factor 1.25, 25 elements
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void red_test() {
	static uint16_t timer;
	
	const int step = 35;
	const int speed = 33;
	
	timer += speed;
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].r = pgm_read_byte(gammasin + t) >> 2;
	}
}	

void green_test() {
	static uint16_t timer;
	
	const int step = 35;
	const int speed = 3;
	
	timer += speed;
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].g = pgm_read_byte(gammasin + t);
	}
}

void blue_test() {
	static uint16_t timer;
	
	const int step = 35;
	const int speed = 3;
	
	timer += speed;
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].b = pgm_read_byte(gammasin + t);
	}
}

void audio_test() {
	pixels[0].r = pixels[0].g = bright1 >> 2;
	pixels[1].r = pixels[1].g = bright1 >> 2;
	pixels[2].r = pixels[2].b = bright2;
	pixels[3].r = pixels[3].g = pixels[3].b = bright2;
	pixels[4].r = pixels[4].g = pixels[4].b = bright2;
	pixels[5].r = pixels[5].b = bright2;
	pixels[6].r = pixels[6].g = bright1 >> 2;
	pixels[7].r = pixels[7].g = bright1 >> 2;
}	

void speed() {
	static uint8_t dir = 0;
	static uint8_t mode = 0;
	static uint16_t timer;
	
	if(mode == 0) {
		if(bright2 > 255-20) {
			mode = 1;
			dir = !dir;
		}			
	} else {
		if(bright2 < 20) {
			mode = 0;
		}			
	}		
	
	const int step = 35;
	
	if(dir) {
		timer += (bright2 >> 2);
	} else {
		timer -= (bright2 >> 2);
	}				
	uint8_t phase = timer >> 8;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].r = pixels[i].b = pgm_read_byte(gammasin + t);
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
		
	pixels[0].g = (32  * bright1) / 256;
	pixels[1].g = (64  * bright1) / 256;
	pixels[2].g = (128 * bright1) / 256;
	pixels[3].g = (255 * bright1) / 256;
	pixels[4].g = (255 * bright1) / 256;
	pixels[5].g = (128 * bright1) / 256;
	pixels[6].g = (64  * bright1) / 256;
	pixels[7].g = (32  * bright1) / 256;

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
	static uint16_t timerR;
	static uint16_t timerG;
	static uint16_t timerB;
	
	const int stepR = 25;
	const int stepG = 26;
	const int stepB = 27;
	const int speedR = -2;
	const int speedG = -3;
	const int speedB = -5;
	
	
	timerR += speedR;
	timerG += speedG;
	timerB += speedB;
	uint8_t phaseR = timerR >> 8;
	uint8_t phaseG = timerG >> 8;
	uint8_t phaseB = timerB >> 8;
	
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
	const int stepG = 26;
	const int stepB = 27;
	const int speedR = -7;
	const int speedG = -8;
	const int speedB = -9;
	
	
	timerR += speedR + (bright1 >> 2);
	timerG += speedG + (bright1 >> 2);
	timerB += speedB + (bright1 >> 2);
	uint8_t phaseR = timerR >> 8;
	uint8_t phaseG = timerG >> 8;
	uint8_t phaseB = timerB >> 8;
	
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
	t = led_tick;
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
		
	t = led_tick;
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

	t = led_tick;
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

void crosscross() {
	/*
	Pixel dark[8] = {
		{   0,   0,   2 },
		{   0,   0,   6 },
		{   2,   0,  11 },
		{   5,   2,  17 },
		{   5,   2,  17 },
		{   2,   0,  11 },
		{   0,   0,   6 },
		{   0,   0,   2 },
	};
	*/

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
	
	const int step = 35;
	const int speed = 77;
	
	timer += speed;
	uint8_t phase = timer >> 8;
	uint8_t cursor = phase;
	for(int i = 0; i < 8; i++) {
		uint8_t r1 = ((dark[i].r * (255-bright2)) + (pulse[i].r * bright2)) >> 8;
		uint8_t g1 = ((dark[i].g * (255-bright2)) + (pulse[i].g * bright2)) >> 8;
		uint8_t b1 = ((dark[i].b * (255-bright2)) + (pulse[i].b * bright2)) >> 8;

		uint8_t r2 = (ears[i].r * bright1) >> 8;
		uint8_t g2 = (ears[i].g * bright1) >> 8;
		uint8_t b2 = (ears[i].b * bright1) >> 8;
		
		uint8_t s = pgm_read_byte(gammasin + cursor);
		cursor += step;
		pixels[i].r = ((r1 + r2) * s) >> 8;
		pixels[i].g = ((g1 + g2) * s) >> 8;
		pixels[i].b = ((b1 + b2 + 16) * s) >> 8;
	}		
}	

void blah() {
	FastWaves();

	for(int i = 0; i < 8; i++) {
		pixels[i].r = (pixels[i].r * bright2) >> 8;
		pixels[i].g = (pixels[i].g * bright2) >> 8;
		pixels[i].b = (pixels[i].b * bright2) >> 8;
	}		
}	