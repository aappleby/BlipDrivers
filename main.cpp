#include "Patterns.h"
#include "LEDDriver.h"
#include "Bobs.h"
#include "Sleep.h"
#include "Tables.h"
#include "Math.h"

#include <math.h>
#include <stdio.h>
#include <stdfix.h>

#define F_CPU 8000000

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

//---------------------------------------------------------------------------

typedef void (*pattern_callback)();

int pattern_index = 0;

pattern_callback patterns[] = {
	CheshireSmile,
  Confetti,
	SunAndStars,
	DancingSapphire,
  hsv_test,
	Blackbody,
	AudioMeter,
	Bliplace1,
	SlowColorCycle,
	PulsingRainbows,
	RomanCandle,
	Fireworks,
  
  /*
	pov_test,
	red_test,
	green_test,
	blue_test,
  */
};


int main(void)
{
  //int16_t x = lerp_s8_s16(ssintab, (16 * 256) + 100);
  
  /*
  for(int i = 0; i < 30000; i++) {
    uint16_t a = xor128();
    int16_t b = xor128();
    
    int32_t x1 = (int32_t(a) * int32_t(b)) >> 16;
    int32_t x2 = mul_us16(a, b);
    
    if(x1 != x2) {
      result = 0;
    }      
  }
  */
  
	SetupLEDs();
  
  const int pattern_count = sizeof(patterns) / sizeof(patterns[0]);
  
	while(1) {
    UpdateSleep();
    
    /*
    if((buttonstate1 == 1) && (debounce_down1 > 256)) {
      pattern_index = (pattern_index + 1) % pattern_count;
      debounce_down1 = 0;
    }
		
		blip_clear();

    const Color grape = Color::fromHex("421C52");
    const Color peach = Color::fromHex("F95");
    const Color hotpink = Color::fromHex("f660ab");
    
    uint16_t phase = blip_tick * 41.3718;
    
    for(int i = 0; i < 8; i++) {
      int16_t b1 = lerp_s8_s16(ssintab, phase);
      uint16_t b2 = b1 + 32768;
      pixels[i] = blip_scale(peach, b2);
    }
    */
    
    /*
    uint16_t phase1 = 49152 + (16 * 256);
    uint16_t phase2 = blip_tick * 55;
    
    for(int i = 0; i < 8; i++) {
      
      int16_t offset = blip_ssin(phase2) / 4;
      
      offset = mul_su16(offset, blip_audio1);
      
      pixels[i] = blip_scale(peach, blip_sin(phase1 + offset));
      
      phase1 += 32 * 256;
    }
    */     
    
    /*
    const Color pine = Color::fromHex("#466d3d");
    const Color peach = Color::fromHex("#ffca7a");
    const Color grape = Color::fromHex("5528b2");
    const Color lemon = Color::fromHex("ffff00");

    pixels[0] = blip_scale(pine, blip_audio1);
    pixels[1] = blip_scale(peach, blip_audio2);
    pixels[2] = blip_lerp(grape, lemon, blip_sin(blip_tick * 10));
    
    pixels[3] = blip_smadd(blip_scale(pine, blip_audio1),
                           blip_scale(grape, blip_audio2));
    */

    patterns[pattern_index]();
		blip_swap();
	}
}


