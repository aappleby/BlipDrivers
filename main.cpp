#include "Patterns.h"
#include "LEDDriver.h"
#include "Bobs.h"
#include "Colors.h"
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
	SlowColorCycle,
	PulsingRainbows,
  Confetti,
	CheshireSmile,
	DancingSapphire,
	SunAndStars,
	RomanCandle,
	Fireworks,
	Bliplace1,
	Blackbody,
	AudioMeter,
  
  /*
	pov_test,
  hsv_test,
	red_test,
	green_test,
	blue_test,
  */
};

int main(void)
{
	SetupLEDs();
  
  const int pattern_count = sizeof(patterns) / sizeof(patterns[0]);
  
	while(1) {
    UpdateSleep();
    
    if((buttonstate1 == 1) && (debounce_down1 > 256)) {
      pattern_index = (pattern_index + 1) % pattern_count;
      debounce_down1 = 0;
    }
		
		clear();
    patterns[pattern_index]();
		blip_swap();
	}
}


