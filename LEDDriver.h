#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void SetupLEDs();

struct Pixel {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

extern struct Pixel pixels[8];
void swap();
void clear();

extern uint32_t led_tick; // 4.096 khz

// Button debounce counters
extern volatile uint8_t buttonstate;
extern volatile uint16_t debounce_up;
extern volatile uint16_t debounce_down;



extern uint8_t bright1;
extern uint8_t bright2;
extern uint16_t tmax1;
extern uint16_t tmax2;

// Brightness cursor, treble channel.
extern uint16_t ibright1;

// Brightness cursor, bass channel.
extern uint16_t ibright2;


#ifdef __cplusplus
}
#endif

inline uint8_t fade (uint8_t a, uint8_t b) {
	return (a * b) >> 8;
}

inline uint8_t crossfade (uint8_t a, uint8_t b, uint8_t alpha) {
	return (a*alpha + b*(255-alpha)) >> 8;
}	


#endif
