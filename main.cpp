#include "Patterns.h"
#include "LEDDriver.h"
#include "Bobs.h"
#include "Sleep.h"
#include "Tables.h"
#include "Math.h"

#include <math.h>
#include <stdio.h>
#include <stdfix.h>

#include <avr/wdt.h>

#define F_CPU 8000000

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

//------------------------------------------------------------------------------

typedef void (*pattern_callback)();

int pattern_index = 0;

pattern_callback patterns[] = {
  Ocean,
  BouncingBalls,
	RomanCandle,
	SlowColorCycle,
	AudioMeter,
	CheshireSmile,
  Confetti,
	SunAndStars,
	DancingSapphire,
  hsv_test,
	Blackbody,
	Bliplace1,
	PulsingRainbows,
	//Fireworks,
  
	pov_test,
	red_test,
	green_test,
	blue_test,
};

extern uint8_t blip_bits_green[8];

const Color grape = Color::fromHex("421C52");
const Color peach = Color::fromHex("F95");
const Color hotpink = Color::fromHex("f660ab");
const Color pine = Color::fromHex("#466d3d");
const Color lemon = Color::fromHex("ffff33");

int main(void)
{
  //blip_selftest();
  
	blip_setup();
  
  const int pattern_count = sizeof(patterns) / sizeof(patterns[0]);
  
	while(1) {
    UpdateSleep();
    
    if((buttonstate1 == 1) && (debounce_down1 > 256)) {
      pattern_index = (pattern_index + 1) % pattern_count;
      debounce_down1 = 0;
    }
    
    /*
		blip_clear();

    uint16_t phase = blip_tick * 11.3718;
    
    for(int i = 0; i < 8; i++) {
      int16_t b1 = lerp_s8_s16(ssintab, phase);
      uint16_t b = b1 + 32768;
      blip_pixels[i] = blip_scale(lemon, b);
      phase += 7000;
    }
    */
    
    //blip_audio_enable = buttonstate2;

    blip_clear();
    patterns[pattern_index]();
		blip_swap();
	}
}


