#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

#include "Config.h"

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
extern volatile uint8_t buttonstate1;
extern volatile uint16_t debounce_up1;
extern volatile uint16_t debounce_down1;

extern volatile uint8_t buttonstate2;
extern volatile uint16_t debounce_up2;
extern volatile uint16_t debounce_down2;



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

#endif
