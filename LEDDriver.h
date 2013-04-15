#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

#include "Config.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
  #endif

void blip_setup();
void TestLEDs();

// Our "framebuffer" is a simple array of 8 RGB blip_pixels.

struct Pixel {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

extern struct Pixel blip_pixels[8];

// Clear the framebuffer to black.
void blip_clear();

// Swap immediately.
void blip_swap();

// Swap w/ a fixed refresh rate of 64 hz.
void blip_swap64();

// Our global 'clock' ticks 4096 times a second.
extern volatile uint32_t blip_tick;

// Button debounce counters
extern volatile uint8_t buttonstate1;
extern volatile uint16_t debounce_up1;
extern volatile uint16_t debounce_down1;

extern volatile uint8_t buttonstate2;
extern volatile uint16_t debounce_up2;
extern volatile uint16_t debounce_down2;

extern uint8_t bright1;
extern uint8_t bright2;
extern uint16_t blip_trigger1;
extern uint16_t blip_trigger2;

extern uint8_t* const blip_history1;
extern uint8_t* const blip_history2;

// Brightness cursor, channel 1.
extern uint16_t blip_audio1;

// Brightness cursor, channel 2.
extern uint16_t blip_audio2;


#ifdef __cplusplus
}
#endif

#endif
