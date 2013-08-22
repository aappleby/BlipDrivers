#include "Bliplace2.h"
#include "Patterns.h"

#include <avr/interrupt.h>

#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>

//------------------------------------------------------------------------------

typedef void (*pattern_callback)();

int pattern_index = 0;

void SleepPattern();

pattern_callback patterns[] = {
	SlowColorCycle,
  Strobey,
  ColorFlip,
	AudioMeter,
  SleepPattern,
  BouncingBalls,
  Ocean,
	SunAndStars,
	Blackbody,
	CheshireSmile,
	RomanCandle,
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

void TempoPattern() {
  uint16_t tempo = blip_tempo(60);
  
  for (int i = 0; i < 8; i++) {
    blip_pixels[i].g = blip_scale(blip_sin(tempo + i * 8192 + 4096), blip_bright1);
  }    
}  

int main(void)
{
	/*
	DDRB = 0xFF;
	PORTB = 0xFB;
	DDRC = 0x00;
	PORTC = 0x00;
	DDRD = 0xFF;
	PORTD = 0x01;
	while(1) {
		PORTB = 0xFD;
		for (int i = 0; i < 8; i++) {
			_delay_ms(1000);
			PORTD = (PORTD << 1) | (PORTD >> 7);		
		}
		PORTB = 0xFE;
		for (int i = 0; i < 8; i++) {
			_delay_ms(1000);
			PORTD = (PORTD << 1) | (PORTD >> 7);
		}
		PORTB = 0xFB;
		for (int i = 0; i < 8; i++) {
			_delay_ms(1000);
			PORTD = (PORTD << 1) | (PORTD >> 7);
		}
	}
	*/
	
	
    //blip_selftest();
  
	blip_setup();
  
  /*
  cli();
  DDRD = 0xFF;
  PORTD = 0xFF;
  while(1);
  */
  
	while(1) {
    
    ///*
    if((buttonstate1 == 1) && (debounce_down1 > 256)) {
      pattern_index++;
      if (pattern_index == pattern_count) pattern_index = 0;
      debounce_down1 = 0;
    }
    //*/

    blip_clear();
    //AudioMeter();
    patterns[pattern_index]();
    //TempoPattern();
    
	  blip_swap();
	}
}
