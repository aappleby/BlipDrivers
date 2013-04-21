#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

#include "Config.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------

#define bit(A)   (1 << A)
#define sbi(p,b) { p |= (unsigned char)bit(b); }
#define cbi(p,b) { p &= (unsigned char)~bit(b); }
#define tbi(p,b) { p ^= (unsigned char)bit(b); }
#define gbi(p,b) (p & (unsigned char)bit(b))

#define lo8(A) (((uint16_t)A) & 0xFF)
#define hi8(A) (((uint16_t)A) >> 8)

//--------------------------------------------------------------------------------

// Install the Bliplace LED drivers & interrupts.
void blip_setup();

// Run a simple self-test of the LEDs. Loops forever.
void blip_selftest();

// Clear the framebuffer to black.
void blip_clear();

// Swap immediately.
void blip_swap();

// Swap w/ a fixed refresh rate of 64 hz.
void blip_swap64();

//--------------------------------------------------------------------------------
// Our "framebuffer" is a simple array of 8 RGB blip_pixels.

struct Pixel {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

extern struct Pixel blip_pixels[8];

//--------------------------------------------------------------------------------

// Audio enable flag. If disabled, audio processing will keep using the previous
// sample.
extern uint8_t blip_audio_enable;

// Raw unfiltered ADC sample.
extern uint16_t blip_sample;

// Our global 'clock' ticks 4096 times a second.
extern volatile uint32_t blip_tick;

// Button debounce counters
extern volatile uint8_t buttonstate1;
extern volatile uint16_t debounce_up1;
extern volatile uint16_t debounce_down1;

extern volatile uint8_t buttonstate2;
extern volatile uint16_t debounce_up2;
extern volatile uint16_t debounce_down2;

extern uint16_t blip_trigger1;
extern uint16_t blip_trigger2;

extern uint8_t* const blip_history1;
extern uint8_t* const blip_history2;

// Brightness cursor, channel 1.
extern uint16_t blip_bright1;

// Brightness cursor, channel 2.
extern uint16_t blip_bright2;

//--------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
