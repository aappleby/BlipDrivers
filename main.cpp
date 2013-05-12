#include "Bliplace.h"
#include "Patterns.h"

//------------------------------------------------------------------------------

typedef void (*pattern_callback)();

int pattern_index = 0;

void SleepPattern();

pattern_callback patterns[] = {
  BouncingBalls,
  Ocean,
	SunAndStars,
	SlowColorCycle,
	Blackbody,
  SleepPattern,
	CheshireSmile,
	RomanCandle,
	AudioMeter,
  Confetti,
	DancingSapphire,
  hsv_test,
	Bliplace1,
	PulsingRainbows,
	red_test,
	green_test,
	blue_test,
};

const int pattern_count = sizeof(patterns) / sizeof(patterns[0]);

void SleepPattern() {
  // Clear the screen.
  blip_clear();
  blip_swap();
  
  // Go to sleep.
  blip_sleep();
  
  // When we wake up, advance to the next pattern so we don't just go back
  // to sleep again.
  pattern_index++;
  if (pattern_index == pattern_count) pattern_index = 0;
}

extern uint8_t blip_bits_green[8];

/*
const Color grape = Color::fromHex("421C52");
const Color peach = Color::fromHex("F95");
const Color hotpink = Color::fromHex("f660ab");
const Color pine = Color::fromHex("#466d3d");
const Color lemon = Color::fromHex("ffff33");
*/

int main(void)
{
  //wdt_reset();
  //wdt_disable();
  
  blip_selftest();
  
	blip_setup();
  
	while(1) {
    
    if((buttonstate1 == 1) && (debounce_down1 > 256)) {
      pattern_index++;
      if (pattern_index == pattern_count) pattern_index = 0;
      debounce_down1 = 0;
    }

    //blip_audio_enable = buttonstate2;

    blip_clear();
    patterns[pattern_index]();
    
    //uint16_t t = blip_tick * 16;
    
    //t = blip_pow6(65536 - t);
    //t = blip_halfsin(t);
    //t = blip_pow4(t);
    
    //blip_pixels[0].r = t;
    //blip_pixels[1].r = blip_sin(blip_tick * 16 - 16384);
    //Color teal = Color::fromHex("#0f8");
    //blip_draw_sin(blip_tick * 16, 8192, teal);
		blip_swap();
	}
}
