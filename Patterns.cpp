#include "Patterns.h"
#include "Tables.h"
#include "LEDDriver.h"

#include <avr/pgmspace.h>
#include <math.h>

//--------------------------------------------------------------------------------
// Tests button functionality.

void button_test() {
	blip_clear();
	for(int i = 0; i < 4; i++) {
		if(buttonstate1) blip_pixels[i].g = 0xFFFF;
		else blip_pixels[i].r = 0xFFFF;
	}
	for(int i = 4; i < 8; i++) {
		if(buttonstate2) blip_pixels[i].g = 0xFFFF;
		else blip_pixels[i].r = 0xFFFF;
	}
}

//--------------------------------------------------------------------------------
// Sine waves in the red channel.

void red_test() {
	uint16_t phase = blip_tick * 16;
	for(int i = 0; i < 8; i++) {
		blip_pixels[i].r = blip_sin(phase);
    phase += 8000;
	}
}	

//--------------------------------------------------------------------------------
// Sine waves in the green channel.

void green_test() {
	uint16_t phase = blip_tick * 16;
	for(int i = 0; i < 8; i++) {
		blip_pixels[i].g = blip_sin(phase);
    phase += 8000;
	}
}

//--------------------------------------------------------------------------------
// Sine waves in the blue channel.

void blue_test() {
	uint16_t phase = blip_tick * 16;
	for(int i = 0; i < 8; i++) {
		blip_pixels[i].b = blip_sin(phase);
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

		blip_pixels[i].r = r;
		blip_pixels[i].g = g;
		blip_pixels[i].b = b;
    
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
	uint16_t bass = blip_bright2 / 64;
  
	for(int i = 3; i >= 0; i--) {
		if(bass > 256) {
			blip_pixels[i].r = 0xFFFF;
			blip_pixels[i].g = 0xFFFF >> 1;
			bass -= 256;
		}
		else 
		{
			blip_pixels[i].r = bass << 8;
			blip_pixels[i].g = bass << 6;
			break;
		}
	}
  
	for(int i = 4; i < 8; i++) {
		if(treb > 256) {
			blip_pixels[i].b = 0xFFFF >> 1;
			blip_pixels[i].g = 0xFFFF;
			treb -= 256;
		}
		else
		{
			blip_pixels[i].b = treb << 6;
			blip_pixels[i].g = treb << 8;
			break;
		}
	}
}

//--------------------------------------------------------------------------------
// Backwards-compatibility mode. :)

void Bliplace1() {
	blip_pixels[0].r = blip_pixels[0].g = blip_pixels[0].b = blip_bright1;
	blip_pixels[1].r = blip_pixels[1].g = blip_pixels[1].b = blip_bright1;
  
	blip_pixels[2].r = blip_pixels[2].g = blip_pixels[2].b = blip_bright2;
	blip_pixels[3].r = blip_pixels[3].g = blip_pixels[3].b = blip_bright2;
	blip_pixels[4].r = blip_pixels[4].g = blip_pixels[4].b = blip_bright2;
	blip_pixels[5].r = blip_pixels[5].g = blip_pixels[5].b = blip_bright2;
  
	blip_pixels[6].r = blip_pixels[6].g = blip_pixels[6].b = blip_bright1;
	blip_pixels[7].r = blip_pixels[7].g = blip_pixels[7].b = blip_bright1;
}	

//--------------------------------------------------------------------------------
// All LEDs pulse in colors that approximate blackbody radiation,
// gradually falling out of sync.

void Blackbody() {
	static uint16_t phases[8];
	
	for(uint8_t i = 0; i < 8; i++) {
		phases[i] += blip_random() & 63;
		uint16_t cursor = phases[i];
    
		blip_pixels[i].r = blip_pow2( blip_halfsin( blip_root2(cursor) ) );
		blip_pixels[i].g = blip_pow3( blip_halfsin( blip_root2(cursor) ) );
		blip_pixels[i].b = blip_pow6( blip_halfsin( blip_root2(cursor) ) );
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
		uint16_t g = blip_pow4(blip_halfsin(blip_root2(phase1)));
		uint16_t b = blip_pow4(blip_halfsin(blip_root2(65535 - phase2)));
		blip_pixels[i].g = g;
		blip_pixels[i].b = b;
    
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

		blip_pixels[i].r = r;
		blip_pixels[i].g = g;
		blip_pixels[i].b = b;
    
    phase_r += 7000;
    phase_g += 8000;
    phase_b += 9000;
	}
}	
 
//--------------------------------------------------------------------------------
// Slow, non-audio-responsive color fading. Good test for LED color mixing
// smoothness.

void SlowColorCycle() {
  uint16_t time = blip_tick;
	for(int i = 0; i < 8; i++) {
		blip_pixels[i].r = blip_sin(time * 3 + i * 6500);
		blip_pixels[i].g = blip_sin(time * 4 + i * 7000);
		blip_pixels[i].b = blip_sin(time * 5 + i * 7500);
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
		blip_pixels[i].r = blip_pixels[i].g = s;

    // blue = noise^3 * audio1;
    uint16_t b = blip_pow3(blip_noise(phase));
    b = blip_scale(b, blip_bright1);
    blip_pixels[i].b = b;
    phase += 40503;
	}		
}

//--------------------------------------------------------------------------------
// Sparks shoot across from the left and split into rainbows.

void RomanCandle() {
	
  uint16_t cursor_r = 0;
  uint16_t cursor_g = 0;
  uint16_t cursor_b = 0;
  
  for(int i = 0; i < 8; i++) {
    blip_pixels[i].r = blip_pow6(blip_history2(cursor_r));
    blip_pixels[i].g = blip_pow6(blip_history2(cursor_g));
    blip_pixels[i].b = blip_pow6(blip_history2(cursor_b));
    
    cursor_r += 192;
    cursor_g += 128;
    cursor_b += 256;
  }
}

//--------------------------------------------------------------------------------

Color pink = Color::fromHex("#F08");
Color teal = Color::fromHex("#0F8");

void BouncingBalls() {  
  // Draw the history from left to right in pink.
  for (uint16_t i = 0; i < 8; i++) {
    uint16_t history = blip_pow5(blip_history2(200 * i));
    blip_pixels[i] = blip_scale(pink, history);
  }
  
  // Then draw the history from right to left in teal and add it to the pink.
  for (uint16_t i = 0; i < 8; i++) {
    uint16_t history = blip_pow5(blip_history2(200 * (7 - i)));
    blip_pixels[i] = blip_smadd(blip_pixels[i], blip_scale(teal, history));
  }    
}

//--------------------------------------------------------------------------------
// Slow blue-green waves that pulse subtly.

void Ocean() {
  const Color darkblue = Color::fromHex("#006");
  const Color darkteal = Color::fromHex("#063");
  const Color skyblue = Color::fromHex("#8CF");

  // Change the time value to change the speed of the waves.
  uint16_t time = blip_tick / 8;
  
  for (int i = 0; i < 8; i++) {
    // Use the noise function to make dark blue and teal waves.
    Color bluewave = blip_scale(darkblue, blip_noise(time + i * 130));
    Color tealwave = blip_scale(darkteal, blip_noise(time - i * 150 + 10000));
    
    // Add the two waves together.
    Color waves = blip_smadd(bluewave, tealwave);
    
    // Use a piece of a sine wave colored sky blue as our "glow".
    Color glow = blip_scale(skyblue, blip_sin(i * 8192 - 12288));
    
    // Blend between the waves and the glow based on the audio, but not all the
    // way so that we can still see some waves.
    blip_pixels[i] = blip_lerp(waves, glow, blip_bright1 * 0.7);
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

	Color center[8] = {
		{  1000,     0,    0 },
		{ 12000,  1000,    0 },
		{ 40000, 17000,    0 },
		{ 65000, 50000, 8000 },
		{ 65000, 50000, 8000 },
		{ 40000, 17000,    0 },
		{ 12000,  1000,    0 },
		{  1000,     0,    0 },
	};

	Color corners[8] = {
		{   0, 32000, 0 },
		{   0, 16000, 0 },
		{   0,  8000, 0 },
		{   0,  4000, 0 },
		{   0,  4000, 0 },
		{   0,  8000, 0 },
		{   0, 16000, 0 },
		{   0, 32000, 0 },
	};

	for(int i = 0; i < 8; i++) {
    Color c1 = blip_scale(center[i], blip_bright2);
    Color c2 = blip_scale(corners[i], blip_bright1);
    
    Color c3 = blip_add(c1, c2);
    c3.b += 32 * 256;
    
    blip_pixels[i].r = c3.r;
    blip_pixels[i].g = c3.g;
    blip_pixels[i].b = c3.b;
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
    
    blip_pixels[i].r = r;
    blip_pixels[i].g = g;
    blip_pixels[i].b = b;

    phase_r += 40503;
    phase_g += 40503;
    phase_b += 40503;
  }    
}  

//--------------------------------------------------------------------------------
