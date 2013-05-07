#include "Patterns.h"
#include "LEDDriver.h"
#include "Tables.h"

#define F_CPU 8000000

//------------------------------------------------------------------------------

typedef void (*pattern_callback)();

int pattern_index = 0;

void SleepPattern();

pattern_callback patterns[] = {
  Ocean,
  SleepPattern,
	CheshireSmile,
  BouncingBalls,
	RomanCandle,
	SlowColorCycle,
	AudioMeter,
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
  
  // Wait for the button to be released so the main loop doesn't respond to
  // the wake-up button press.
  while(buttonstate1 == 0);
  debounce_down1 = 0;
}

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
  
  
	while(1) {
    
    if((buttonstate1 == 1) && (debounce_down1 > 256)) {
      pattern_index++;
      if (pattern_index == pattern_count) pattern_index = 0;
      debounce_down1 = 0;
    }

    //blip_audio_enable = buttonstate2;

    blip_clear();
    patterns[pattern_index]();
		blip_swap();
	}
}

extern "C" {
void exit() {
}
};  