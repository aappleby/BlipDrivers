#include "Patterns.h"
#include "Tables.h"
#include "LEDDriver.h"
#include "Math.h"

#include <avr/pgmspace.h>
#include <math.h>

//-----------------------------------------------------------------------------
// Tests button functionality.

void button_test() {
	blip_clear();
	for(int i = 0; i < 4; i++) {
		if(buttonstate1) pixels[i].g = 0xFF;
		else pixels[i].r = 0xFF;
	}
	for(int i = 4; i < 8; i++) {
		if(buttonstate2) pixels[i].g = 0xFF;
		else pixels[i].r = 0xFF;
	}
}

//-----------------------------------------------------------------------------
// Sine waves in the red channel.

void red_test() {
	uint16_t phase = blip_tick * 16;
	for(int i = 0; i < 8; i++) {
    uint16_t r = blip_sin(phase);
		pixels[i].r = blip_gamma(r);
    phase += 8000;
	}
}	

//-----------------------------------------------------------------------------
// Sine waves in the green channel.

void green_test() {
	uint16_t phase = blip_tick * 16;
	for(int i = 0; i < 8; i++) {
    uint16_t g = blip_sin(phase);
		pixels[i].g = blip_gamma(g);
    phase += 8000;
	}
}

//-----------------------------------------------------------------------------
// Sine waves in the blue channel.

void blue_test() {
	uint16_t phase = blip_tick * 16;
	for(int i = 0; i < 8; i++) {
    uint16_t b = blip_sin(phase);
		pixels[i].b = blip_gamma(b);
    phase += 8000;
	}
}

//-----------------------------------------------------------------------------
// Scrolling rainbow, tests table interpolation.

void hsv_test() {
	uint16_t phase = blip_tick * 5;

	for(int i = 0; i < 8; i++) {
    uint16_t r = blip_hsv_r(phase);
    uint16_t g = blip_hsv_g(phase);
    uint16_t b = blip_hsv_b(phase);

		pixels[i].r = blip_gamma(r);
		pixels[i].g = blip_gamma(g);
		pixels[i].b = blip_gamma(b);
    
    phase += 5000;
	}
}

//-----------------------------------------------------------------------------
// VU-meter mode, tests the microphone & allows for direct visualization of
// the audio intensity. The left 4 LEDs show the bass intensity, the right 4
// show the treble intensity.

void AudioMeter() {
  // Divide the 16-bit intensity values down into the (0,1023) range.
	uint16_t treb = blip_audio1 / 64;
	uint16_t bass = blip_audio2 / 64;
  
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

//-----------------------------------------------------------------------------
// Backwards-compatibility mode. :)

void Bliplace1() {
  uint8_t b1 = blip_gamma(blip_audio1);
  uint8_t b2 = blip_gamma(blip_audio2);
  
	pixels[0].r = pixels[0].g = pixels[0].b = b1;
	pixels[1].r = pixels[1].g = pixels[1].b = b1;
  
	pixels[2].r = pixels[2].g = pixels[2].b = b2;
	pixels[3].r = pixels[3].g = pixels[3].b = b2;
	pixels[4].r = pixels[4].g = pixels[4].b = b2;
	pixels[5].r = pixels[5].g = pixels[5].b = b2;
  
	pixels[6].r = pixels[6].g = pixels[6].b = b1;
	pixels[7].r = pixels[7].g = pixels[7].b = b1;
}	

//-----------------------------------------------------------------------------
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
    
		pixels[i].r = blip_gamma(r);
		pixels[i].g = blip_gamma(g);
		pixels[i].b = blip_gamma(b);
	}
}

//-----------------------------------------------------------------------------
// Blue & green pulses that move back and forth with the beat.

void DancingSapphire() {
	static uint8_t dir = 0;
	static bool high = false;
	
	if(high) {
		if(blip_audio2 < 8192) {
			high = false;
		}
	} else {
		if(blip_audio2 > 16384 + 8192) {
			high = true;
			dir = !dir;
		}
	}
	
	static uint16_t timer1;
	static uint16_t timer2;
	if(dir) {
		timer1 += (blip_audio1 >> 10);
		timer2 += (blip_audio2 >> 10);
	} else {
		timer1 -= (blip_audio1 >> 10);
		timer2 -= (blip_audio2 >> 10);
	}
  
	uint16_t phase1 = timer1;
	uint16_t phase2 = timer2;

	for(int i = 0; i < 8; i++) {
		uint16_t g = lerp_u8_u16(pulse_2_4, phase1);
		uint16_t b = lerp_u8_u16(pulse_2_4, 65535 - phase2);
		pixels[i].g = blip_gamma(g);
		pixels[i].b = blip_gamma(b);
    
    phase1 += 8192;
    phase2 += 8192;
	}
}

//-----------------------------------------------------------------------------
// Medium-speed rainbow sine waves that react to sound - the green
// channel blinks with treble, the red and blue with bass.

void PulsingRainbows() {
	uint16_t phase_r = blip_tick * 12;
	uint16_t phase_g = -blip_tick * 9;
	uint16_t phase_b = blip_tick * 15;

	for(int i = 0; i < 8; i++) {
    uint16_t r = blip_scale(blip_sin(phase_r), blip_audio1);
    uint16_t g = blip_scale(blip_sin(phase_g), blip_audio2);
    uint16_t b = blip_scale(blip_sin(phase_b), blip_audio1);

		pixels[i].r = blip_gamma(r);
		pixels[i].g = blip_gamma(g);
		pixels[i].b = blip_gamma(b);
    
    phase_r += 7000;
    phase_g += 8000;
    phase_b += 9000;
	}
}	
 
//-----------------------------------------------------------------------------
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

		pixels[i].r = blip_gamma(r);
		pixels[i].g = blip_gamma(g);
		pixels[i].b = blip_gamma(b);
    
    phase_r += 6500;
    phase_g += 7000;
    phase_b += 7500;
	}
}	

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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
    s = blip_scale(s, blip_audio2);
		pixels[i].r = pixels[i].g = blip_gamma(s);

    // blue = noise^3 * audio1;
    uint16_t b = blip_pow3(blip_noise(phase));
    b = blip_scale(b, blip_audio1);
    pixels[i].b = blip_gamma(b);
    phase += 40503;
	}		
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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
    Color c1 = blip_scale(center[i], blip_audio2);
    Color c2 = blip_scale(corners[i], blip_audio1);
    
    Color c3 = c1 + c2;
    c3.b += 32 * 256;
    
    pixels[i].r = blip_gamma(c3.r);
    pixels[i].g = blip_gamma(c3.g);
    pixels[i].b = blip_gamma(c3.b);
	}
}	

//-----------------------------------------------------------------------------
// Glittery rainbow noise.

void Confetti() {
  uint16_t phase_r = blip_tick * 0.75;
  uint16_t phase_g = blip_tick * 1.25;
  uint16_t phase_b = blip_tick * 1.80;
  
  for(int i = 0; i < 8; i++) {
    uint16_t r = blip_noise(phase_r);
    uint16_t g = blip_noise(phase_g);
    uint16_t b = blip_noise(phase_b);
    
    r = blip_scale(r, blip_audio1);
    g = blip_scale(g, blip_audio2);
    b = blip_scale(b, blip_audio2);
    
    pixels[i].r = blip_gamma(r);
    pixels[i].g = blip_gamma(g);
    pixels[i].b = blip_gamma(b);

    phase_r += 40503;
    phase_g += 40503;
    phase_b += 40503;
  }    
}  

//-----------------------------------------------------------------------------
