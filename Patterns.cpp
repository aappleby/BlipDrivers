#include "Patterns.h"
#include "Tables.h"
#include "LEDDriver.h"
#include "Math.h"

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

// Tests button functionality.
void button_test() {
	clear();
	for(int i = 0; i < 4; i++) {
		if(buttonstate1) pixels[i].g = 0xFF;
		else pixels[i].r = 0xFF;
	}
	for(int i = 4; i < 8; i++) {
		if(buttonstate2) pixels[i].g = 0xFF;
		else pixels[i].r = 0xFF;
	}
}

// Sine waves in the red channel.
void red_test() {
	uint8_t phase = blip_tick / 16;
	const int step = 35;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].r = pgm_read_byte(gammasin + t);
	}
}	

// Sine waves in the green channel.
void green_test() {
	uint8_t phase = blip_tick / 16;
	const int step = 35;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].g = pgm_read_byte(gammasin + t);
	}
}

// Sine waves in the blue channel.
void blue_test() {
	uint8_t phase = blip_tick / 16;
	const int step = 35;

	for(int i = 0; i < 8; i++) {
		uint8_t t = phase + step * i;
		pixels[i].b = pgm_read_byte(gammasin + t);
	}
}

// Scrolling rainbow, tests table interpolation.
void hsv_test() {
	uint16_t phase = blip_tick * 8;
	uint16_t step = 32 * 256;

  uint16_t s, b;

	for(int i = 0; i < 8; i++) {
    s = tablelerp_asm3(huetab, phase);
    b = tablelerp_asm3_nowrap(cielum, s);
		pixels[i].r = b >> 8;
    
    s = tablelerp_asm3(huetab, phase + 85 * 256);
    b = tablelerp_asm3_nowrap(cielum, s);
		pixels[i].g = b >> 8;

    s = tablelerp_asm3(huetab, phase + uint16_t(170) * uint16_t(256));
    b = tablelerp_asm3_nowrap(cielum, s);
		pixels[i].b = b >> 8;
    
    phase += step;
	}
}

// VU-meter mode, tests the microphone & allows for direct visualization of
// the audio intensity. The left 4 LEDs show the bass intensity, the right 4
// show the treble intensity.
void AudioMeter() {
	clear();
  // Divide the 16-bit intensity values down into the (0,1023) range.
	uint16_t treb = ibright1 / 64;
	uint16_t bass = ibright2 / 64;
  
	for(int i = 3; i >= 0; i--) {
		if(bass > 256) {
			pixels[i].r = 0xFF;
			pixels[i].g = 0xFF >> 2;
			bass -= 256;
		}
		else 
		{
			bass = (bass * bass) >> 8;
			pixels[i].r = bass;
			pixels[i].g = bass >> 2;
			break;
		}
	}
  
	for(int i = 4; i < 8; i++) {
		if(treb > 256) {
			pixels[i].b = 0xFF >> 2;
			pixels[i].g = 0xFF;
			treb -= 256;
		}
		else
		{
			treb = (treb * treb) >> 8;
			pixels[i].b = treb >> 2;
			pixels[i].g = treb;
			break;
		}
	}
}

// Backwards-compatibility mode. :)
void Bliplace1() {
	pixels[0].r = pixels[0].g = pixels[0].b = bright1;
	pixels[1].r = pixels[1].g = pixels[1].b = bright1;
  
	pixels[2].r = pixels[2].g = pixels[2].b = bright2;
	pixels[3].r = pixels[3].g = pixels[3].b = bright2;
	pixels[4].r = pixels[4].g = pixels[4].b = bright2;
	pixels[5].r = pixels[5].g = pixels[5].b = bright2;
  
	pixels[6].r = pixels[6].g = pixels[6].b = bright1;
	pixels[7].r = pixels[7].g = pixels[7].b = bright1;
}	

// All LEDs pulse in colors that approximate blackbody radiation,
// gradually falling out of sync.
void Blackbody() {
	static uint16_t phases[8];
	
	for(uint8_t i = 0; i < 8; i++) {
		phases[i] += rng() & 3;
		uint8_t cursor = phases[i] >> 4;
		pixels[i].r = pgm_read_byte(pulse_2_2 + cursor);
		pixels[i].g = pgm_read_byte(pulse_2_3 + cursor);
		pixels[i].b = pgm_read_byte(pulse_2_6 + cursor);
	}
}

// Blue & green pulses that move back and forth with the beat.
void DancingSapphire() {
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

// Medium-speed rainbow sine waves that react to sound - the green
// channel blinks with treble, the red and blue with bass.
void PulsingRainbows() {
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
 
// Slow, non-audio-responsive color fading. Good test for LED color mixing
// smoothness.
void SlowColorCycle() {
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

// Displays very fast-moving sine waves in all three color channels,
// serves as a crude persistence-of-vision effect.
void pov_test() {
	static uint16_t timerR;
	static uint16_t timerG;
	static uint16_t timerB;
	
	const int stepR = 25;
	const int stepG = 36;
	const int stepB = 37;
	const int speedR = -327;
	const int speedG = -528;
	const int speedB = 629;
	
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
		pixels[i].r = pgm_read_byte(gammapulse + cursorR);
		pixels[i].g = pgm_read_byte(gammapulse + cursorG);
		pixels[i].b = pgm_read_byte(gammapulse + cursorB);
		
		cursorR += stepR;
		cursorG += stepG;
		cursorB += stepB;
	}
}

// Blue sparkles that react to treble, big yellow blob in the middle
// that reacts to bass.
void SunAndStars() {
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

// Sparks shoot across from the left and split into rainbows.
void RomanCandle() {
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

// Sparks explode and split into colors. The 'explosion' source slowly moves
// from side to side as well.
void Fireworks() {
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


// 'Corners' of the mouth light up green with treble, center lights up
// yellow-white with bass, dim blue background. Good audio visualization.
void CheshireSmile() {

	Pixel center[8] = {
		{   5,   0,   0 },
		{  48,   5,   0 },
		{ 128,  48,   0 },
		{ 255, 192,  32 },
		{ 255, 192,  32 },
		{ 128,  48,   0 },
		{  48,   5,   0 },
		{   5,   0,   0 },
	};

	Pixel corners[8] = {
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
	for(int i = 0; i < 8; i++) {
		uint8_t r1 = (center[i].r * bright2) >> 8;
		uint8_t g1 = (center[i].g * bright2) >> 8;
		uint8_t b1 = (center[i].b * bright2) >> 8;

		uint8_t r2 = (corners[i].r * bright1) >> 8;
		uint8_t g2 = (corners[i].g * bright1) >> 8;
		uint8_t b2 = (corners[i].b * bright1) >> 8;
		
		pixels[i].r = (r1 + r2);
		pixels[i].g = (g1 + g2);
		pixels[i].b = (b1 + b2 + 8);
	}		
}	

// Glittery rainbow noise.
void Confetti() {
  const uint16_t step = 157 << 8;
  uint16_t phase_r = (blip_tick * 3) >> 2;
  uint16_t phase_g = (blip_tick * 5) >> 2;
  uint16_t phase_b = (blip_tick * 2) >> 2;
  
  for(int i = 0; i < 8; i++) {
    uint8_t r = tablelerp8_asm(noise, phase_r);
    uint8_t g = tablelerp8_asm(noise, phase_g);
    uint8_t b = tablelerp8_asm(noise, phase_b);
    
    r = (r * r) >> 8;
    g = (g * g) >> 8;
    b = (b * b) >> 8;

    pixels[i].r = (r * bright1) >> 8;
    pixels[i].g = (g * bright2) >> 8;
    pixels[i].b = (b * bright2) >> 8;

    phase_r += step;
    phase_g += step;
    phase_b += step;
  }    
}  

