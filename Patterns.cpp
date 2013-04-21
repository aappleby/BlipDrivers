#include "Patterns.h"
#include "Tables.h"
#include "LEDDriver.h"
#include "Math.h"

#include <avr/pgmspace.h>
#include <math.h>

//--------------------------------------------------------------------------------
// Tests button functionality.

void button_test() {
	blip_clear();
	for(int i = 0; i < 4; i++) {
		if(buttonstate1) blip_pixels[i].g = 0xFF;
		else blip_pixels[i].r = 0xFF;
	}
	for(int i = 4; i < 8; i++) {
		if(buttonstate2) blip_pixels[i].g = 0xFF;
		else blip_pixels[i].r = 0xFF;
	}
}

//--------------------------------------------------------------------------------
// Sine waves in the red channel.

void red_test() {
	uint16_t phase = blip_tick * 16;
	for(int i = 0; i < 8; i++) {
    uint16_t r = blip_sin(phase);
		blip_pixels[i].r = blip_gamma(r);
    phase += 8000;
	}
}	

//--------------------------------------------------------------------------------
// Sine waves in the green channel.

void green_test() {
	uint16_t phase = blip_tick * 16;
	for(int i = 0; i < 8; i++) {
    uint16_t g = blip_sin(phase);
		blip_pixels[i].g = blip_gamma(g);
    phase += 8000;
	}
}

//--------------------------------------------------------------------------------
// Sine waves in the blue channel.

void blue_test() {
	uint16_t phase = blip_tick * 16;
	for(int i = 0; i < 8; i++) {
    uint16_t b = blip_sin(phase);
		blip_pixels[i].b = blip_gamma(b);
    phase += 8000;
	}
}

//--------------------------------------------------------------------------------
// Scrolling rainbow, tests table interpolation.

void hsv_test() {
	uint16_t phase = blip_tick * 5;

	for(int i = 0; i < 8; i++) {
    uint16_t r = blip_hsv_r(phase);
    uint16_t g = blip_hsv_g(phase);
    uint16_t b = blip_hsv_b(phase);

		blip_pixels[i].r = blip_gamma(r);
		blip_pixels[i].g = blip_gamma(g);
		blip_pixels[i].b = blip_gamma(b);
    
    phase += 5000;
	}
}

//--------------------------------------------------------------------------------
// VU-meter mode, tests the microphone & allows for direct visualization of
// the audio intensity. The left 4 LEDs show the bass intensity, the right 4
// show the treble intensity.

void AudioMeter() {
  // Divide the 16-bit intensity values down into the (0,1023) range.
	uint16_t treb = blip_bright1 / 64;
	uint16_t blip_sample2 = blip_bright2 / 64;
  
	for(int i = 3; i >= 0; i--) {
		if(blip_sample2 > 256) {
			blip_pixels[i].r = 0xFF;
			blip_pixels[i].g = 0xFF >> 2;
			blip_sample2 -= 256;
		}
		else 
		{
			blip_sample2 = (blip_sample2 * blip_sample2) >> 8;
			blip_pixels[i].r = blip_sample2;
			blip_pixels[i].g = blip_sample2 >> 2;
			break;
		}
	}
  
	for(int i = 4; i < 8; i++) {
		if(treb > 256) {
			blip_pixels[i].b = 0xFF >> 2;
			blip_pixels[i].g = 0xFF;
			treb -= 256;
		}
		else
		{
			treb = (treb * treb) >> 8;
			blip_pixels[i].b = treb >> 2;
			blip_pixels[i].g = treb;
			break;
		}
	}
}

//--------------------------------------------------------------------------------
// Backwards-compatibility mode. :)

void Bliplace1() {
  uint8_t b1 = blip_gamma(blip_bright1);
  uint8_t b2 = blip_gamma(blip_bright2);
  
	blip_pixels[0].r = blip_pixels[0].g = blip_pixels[0].b = b1;
	blip_pixels[1].r = blip_pixels[1].g = blip_pixels[1].b = b1;
  
	blip_pixels[2].r = blip_pixels[2].g = blip_pixels[2].b = b2;
	blip_pixels[3].r = blip_pixels[3].g = blip_pixels[3].b = b2;
	blip_pixels[4].r = blip_pixels[4].g = blip_pixels[4].b = b2;
	blip_pixels[5].r = blip_pixels[5].g = blip_pixels[5].b = b2;
  
	blip_pixels[6].r = blip_pixels[6].g = blip_pixels[6].b = b1;
	blip_pixels[7].r = blip_pixels[7].g = blip_pixels[7].b = b1;
}	

//--------------------------------------------------------------------------------
// All LEDs pulse in colors that approximate blackbody radiation,
// gradually falling out of sync.

void Blackbody() {
	static uint16_t phases[8];
	
	for(uint8_t i = 0; i < 8; i++) {
		phases[i] += xor128() & 63;
		uint16_t cursor = phases[i];
    
    uint16_t r = lerp_u8_u16(pulse_2_2, cursor);
    uint16_t g = lerp_u8_u16(pulse_2_3, cursor);
    uint16_t b = lerp_u8_u16(pulse_2_6, cursor);
    
		blip_pixels[i].r = blip_gamma(r);
		blip_pixels[i].g = blip_gamma(g);
		blip_pixels[i].b = blip_gamma(b);
	}
}

//--------------------------------------------------------------------------------
// Blue & green pulses that move back and forth with the beat.

void DancingSapphire() {
	static uint8_t dir = 0;
	static bool high = false;
	
	if(high) {
		if(blip_bright2 < 8192) {
			high = false;
		}
	} else {
		if(blip_bright2 > 16384 + 8192) {
			high = true;
			dir = !dir;
		}
	}
	
	static uint16_t timer1;
	static uint16_t timer2;
	if(dir) {
		timer1 += (blip_bright1 >> 10);
		timer2 += (blip_bright2 >> 10);
	} else {
		timer1 -= (blip_bright1 >> 10);
		timer2 -= (blip_bright2 >> 10);
	}
  
	uint16_t phase1 = timer1;
	uint16_t phase2 = timer2;

	for(int i = 0; i < 8; i++) {
		uint16_t g = lerp_u8_u16(pulse_2_4, phase1);
		uint16_t b = lerp_u8_u16(pulse_2_4, 65535 - phase2);
		blip_pixels[i].g = blip_gamma(g);
		blip_pixels[i].b = blip_gamma(b);
    
    phase1 += 8192;
    phase2 += 8192;
	}
}

//--------------------------------------------------------------------------------
// Medium-speed rainbow sine waves that react to sound - the green
// channel blinks with treble, the red and blue with bass.

void PulsingRainbows() {
	uint16_t phase_r = blip_tick * 12;
	uint16_t phase_g = -blip_tick * 9;
	uint16_t phase_b = blip_tick * 15;

	for(int i = 0; i < 8; i++) {
    uint16_t r = blip_scale(blip_sin(phase_r), blip_bright1);
    uint16_t g = blip_scale(blip_sin(phase_g), blip_bright2);
    uint16_t b = blip_scale(blip_sin(phase_b), blip_bright1);

		blip_pixels[i].r = blip_gamma(r);
		blip_pixels[i].g = blip_gamma(g);
		blip_pixels[i].b = blip_gamma(b);
    
    phase_r += 7000;
    phase_g += 8000;
    phase_b += 9000;
	}
}	
 
//--------------------------------------------------------------------------------
// Slow, non-audio-responsive color fading. Good test for LED color mixing
// smoothness.

void SlowColorCycle() {
	uint16_t phase_r = blip_tick * 3;
	uint16_t phase_g = blip_tick * 4;
	uint16_t phase_b = blip_tick * 5;

	for(int i = 0; i < 8; i++) {
    uint16_t r = blip_sin(phase_r);
    uint16_t g = blip_sin(phase_g);
    uint16_t b = blip_sin(phase_b);

		blip_pixels[i].r = blip_gamma(r);
		blip_pixels[i].g = blip_gamma(g);
		blip_pixels[i].b = blip_gamma(b);
    
    phase_r += 6500;
    phase_g += 7000;
    phase_b += 7500;
	}
}	

//--------------------------------------------------------------------------------
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
		blip_pixels[i].r = pgm_read_byte(pulse_5_6 + cursorR);
		blip_pixels[i].g = pgm_read_byte(pulse_5_6 + cursorG);
		blip_pixels[i].b = pgm_read_byte(pulse_5_6 + cursorB);
		
		cursorR += stepR;
		cursorG += stepG;
		cursorB += stepB;
	}
}

//--------------------------------------------------------------------------------
// Blue sparkles that react to treble, big yellow blob in the middle
// that reacts to bass.

void SunAndStars() {
  uint16_t phase = blip_tick / 2;
  
  const uint8_t sun[] = {
    5, 48, 128, 255, 255, 128, 48, 5
  };
	
	for(int i = 0; i < 8; i++) {
    // red = green = sun * audio2;
    uint16_t s = sun[i] << 8;
    s = blip_scale(s, blip_bright2);
		blip_pixels[i].r = blip_pixels[i].g = blip_gamma(s);

    // blue = noise^3 * audio1;
    uint16_t b = blip_pow3(blip_noise(phase));
    b = blip_scale(b, blip_bright1);
    blip_pixels[i].b = blip_gamma(b);
    phase += 40503;
	}		
}

//--------------------------------------------------------------------------------
// Sparks shoot across from the left and split into rainbows.

void RomanCandle() {
	
  uint16_t cursor_r = blip_tick;
  uint16_t cursor_g = cursor_r;
  uint16_t cursor_b = cursor_r;
  
  for(int i = 0; i < 8; i++) {
    blip_pixels[i].r = blip_gamma(blip_pow2(lerp_u8_u16_ram(blip_history2, cursor_r)));
    blip_pixels[i].g = blip_gamma(blip_pow2(lerp_u8_u16_ram(blip_history2, cursor_g)));
    blip_pixels[i].b = blip_gamma(blip_pow2(lerp_u8_u16_ram(blip_history2, cursor_b)));
    
    cursor_r -= 192;
    cursor_g -= 128;
    cursor_b -= 256;
  }    
}

//--------------------------------------------------------------------------------
// Sparks explode and split into colors. The 'explosion' source slowly moves
// from side to side as well.

/*
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
    uint8_t bright = (blip_bright2 >> 8);
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
		
		blip_pixels[i].r = buffer[t1];
		blip_pixels[i].g = buffer[t2];
		blip_pixels[i].b = buffer[t3];
	}
}
*/

//--------------------------------------------------------------------------------
// 'Corners' of the mouth light up green with treble, center lights up
// yellow-white with bass, dim blue background. Good audio visualization.

void CheshireSmile() {

	Pixel center[8] = {
		{   5,   0,   0 },
		{  48,   5,   0 },
		{ 158,  68,   0 },
		{ 255, 192,  32 },
		{ 255, 192,  32 },
		{ 128,  68,   0 },
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

	for(int i = 0; i < 8; i++) {
    Color c1 = blip_scale(center[i], blip_bright2);
    Color c2 = blip_scale(corners[i], blip_bright1);
    
    Color c3 = c1 + c2;
    c3.b += 32 * 256;
    
    blip_pixels[i].r = blip_gamma(c3.r);
    blip_pixels[i].g = blip_gamma(c3.g);
    blip_pixels[i].b = blip_gamma(c3.b);
	}
}	

//--------------------------------------------------------------------------------
// Glittery rainbow noise.

void Confetti() {
  uint16_t phase_r = blip_tick * 0.75;
  uint16_t phase_g = blip_tick * 1.25;
  uint16_t phase_b = blip_tick * 1.80;
  
  for(int i = 0; i < 8; i++) {
    uint16_t r = blip_noise(phase_r);
    uint16_t g = blip_noise(phase_g);
    uint16_t b = blip_noise(phase_b);
    
    r = blip_scale(r, blip_bright1);
    g = blip_scale(g, blip_bright2);
    b = blip_scale(b, blip_bright2);
    
    blip_pixels[i].r = blip_gamma(r);
    blip_pixels[i].g = blip_gamma(g);
    blip_pixels[i].b = blip_gamma(b);

    phase_r += 40503;
    phase_g += 40503;
    phase_b += 40503;
  }    
}  

//--------------------------------------------------------------------------------
