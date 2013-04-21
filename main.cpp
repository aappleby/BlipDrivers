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
	AudioMeter,
	SlowColorCycle,
	RomanCandle,
	CheshireSmile,
  Confetti,
	SunAndStars,
	DancingSapphire,
  hsv_test,
	Blackbody,
	Bliplace1,
	PulsingRainbows,
	//Fireworks,
  
  /*
	pov_test,
	red_test,
	green_test,
	blue_test,
  */
};

extern uint8_t blip_bits_green[8];

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

    const Color grape = Color::fromHex("421C52");
    const Color peach = Color::fromHex("F95");
    const Color hotpink = Color::fromHex("f660ab");
    
    uint16_t phase = blip_tick * 41.3718;
    
    for(int i = 0; i < 8; i++) {
      int16_t b1 = lerp_s8_s16(ssintab, phase);
      uint16_t b2 = b1 + 32768;
      blip_pixels[i] = blip_scale(peach, b2);
    }
    */
    
    /*
    uint16_t phase1 = 49152 + (16 * 256);
    uint16_t phase2 = blip_tick * 55;
    
    for(int i = 0; i < 8; i++) {
      
      int16_t offset = blip_ssin(phase2) / 4;
      
      offset = mul_su16(offset, blip_bright1);
      
      blip_pixels[i] = blip_scale(peach, blip_sin(phase1 + offset));
      
      phase1 += 32 * 256;
    }
    */     
    
    /*
    const Color pine = Color::fromHex("#466d3d");
    const Color peach = Color::fromHex("#ffca7a");
    const Color grape = Color::fromHex("5528b2");
    const Color lemon = Color::fromHex("ffff00");

    blip_pixels[0] = blip_scale(pine, blip_bright1);
    blip_pixels[1] = blip_scale(peach, blip_bright2);
    blip_pixels[2] = blip_lerp(grape, lemon, blip_sin(blip_tick * 10));
    
    blip_pixels[3] = blip_smadd(blip_scale(pine, blip_bright1),
                           blip_scale(grape, blip_bright2));
    */

    blip_audio_enable = buttonstate2;

    blip_clear();
    patterns[pattern_index]();
		blip_swap();
    //blip_bits_green[5] = blip_trigger1;
	}
}


