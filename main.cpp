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

#define bit(A)   (1 << A)
#define sbi(p,b) { p |= (unsigned char)bit(b); }
#define cbi(p,b) { p &= (unsigned char)~bit(b); }

//---------------------------------------------------------------------------
// Minimalist LED test, just verifies that each LED can be addressed
// independently.

void testLEDs() {
	// Turn off the serial interface, which the bootloader leaves on by default.
	UCSR0B &= ~(1 << RXEN0);
	UCSR0B &= ~(1 << TXEN0);
	
	DDRB = 0xFF;
	PORTB = 0x00;
	
	DDRD = 0xFF;
	PORTD = 0x01;
	
	while(1)
	{
    PORTB = SINK_RED;
    for(int i = 0; i < 8; i++) {
      _delay_ms(200);
		  PORTD = (PORTD << 1) | (PORTD >> 7);
    }
    
    PORTB = SINK_GREEN;
    for(int i = 0; i < 8; i++) {
      _delay_ms(200);
		  PORTD = (PORTD << 1) | (PORTD >> 7);
    }

    PORTB = SINK_BLUE;
    for(int i = 0; i < 8; i++) {
      _delay_ms(200);
		  PORTD = (PORTD << 1) | (PORTD >> 7);
    }
	}	
}

//---------------------------------------------------------------------------

typedef void (*pattern_callback)();

int pattern_index = 0;

pattern_callback patterns[] = {
	PulsingRainbows,
  Confetti,
	CheshireSmile,
	DancingSapphire,
	SunAndStars,
	RomanCandle,
	Fireworks,
	Bliplace1,
	Blackbody,
	SlowColorCycle,
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


