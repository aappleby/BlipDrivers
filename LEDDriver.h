#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

#include "Config.h"

#include <stdint.h>

//--------------------------------------------------------------------------------

#define bit(A)   (1 << A)
#define sbi(p,b) { p |= (unsigned char)bit(b); }
#define cbi(p,b) { p &= (unsigned char)~bit(b); }
#define tbi(p,b) { p ^= (unsigned char)bit(b); }
#define gbi(p,b) (p & (unsigned char)bit(b))

#define lo8(A) (((uint16_t)A) & 0xFF)
#define hi8(A) (((uint16_t)A) >> 8)

//--------------------------------------------------------------------------------
// Our "framebuffer" is a simple array of 8 RGB pixels.

struct Color {
  uint16_t r;
  uint16_t g;
  uint16_t b;

  static Color fromRGB(uint16_t r, uint16_t g, uint16_t b);
  static Color fromRGB(int r, int g, int b);
  static Color fromRGB(uint8_t r, uint8_t g, uint8_t b);
  static Color fromRGB(float r, float g, float b);

  static Color fromHue(uint16_t h);
  static Color fromHex(const char* hexcode);
  
  Color operator * (uint16_t scale);    
};

extern Color blip_pixels[8];

//--------------------------------------------------------------------------------

// Install the Bliplace LED drivers & interrupts.
void blip_setup();

// Turn off all peripherals, disable all interrupts.
void blip_shutdown();

// Put the Bliplace to sleep - it will wake up (and return from this function
// call) when the button has been pressed for 1/8 second.
void blip_sleep(uint8_t sink);

// Run a simple self-test of the LEDs. Loops forever.
void blip_selftest();

// Clear the framebuffer to black.
void blip_clear();

// Display the contents of blip_pixels immediately.
void blip_swap();

// Display the contents of blip_pixels, but synchronize with the global timer
// so that the LEDs will change at a maximum rate of 64 hertz.
void blip_swap64();

//--------------------------------------------------------------------------------

uint16_t blip_sin(uint16_t x);
uint16_t blip_cos(uint16_t x);
uint16_t blip_halfsin(uint16_t x);

int16_t  blip_ssin(uint16_t x);
int16_t  blip_scos(uint16_t x);

uint16_t blip_hsv_r(uint16_t h);
uint16_t blip_hsv_g(uint16_t h);
uint16_t blip_hsv_b(uint16_t h);

uint16_t blip_pow2(uint16_t x);
uint16_t blip_pow3(uint16_t x);
uint16_t blip_pow4(uint16_t x);
uint16_t blip_pow5(uint16_t x);
uint16_t blip_pow6(uint16_t x);
uint16_t blip_pow7(uint16_t x);

uint16_t blip_root2(uint16_t x);
uint16_t blip_root3(uint16_t x);
uint16_t blip_root4(uint16_t x);
uint16_t blip_root6(uint16_t x);
uint16_t blip_root9(uint16_t x);

uint16_t blip_noise(uint16_t x);
uint16_t blip_random();

uint16_t blip_scale(uint16_t x, uint16_t s);
int16_t  blip_scale(int16_t x, uint16_t s);
uint16_t blip_smadd(uint16_t a, uint16_t b);

uint16_t blip_lookup(const uint8_t* table, uint16_t x);

uint16_t blip_history1(uint16_t x);
uint16_t blip_history2(uint16_t x);

//--------------------------------------------------------------------------------

Color    blip_scale(Color const& c, uint16_t s);
Color    blip_scale(Color const& c, uint16_t s1, uint16_t s2);
Color    blip_add(Color const& a, Color const& b);
Color    blip_smadd(Color const& a, Color const& b);
Color    blip_lerp(Color const& a, Color const& b, uint16_t x);
Color    blip_hsv(uint16_t h);

//--------------------------------------------------------------------------------

void blip_draw_sin(uint16_t phase, uint16_t frequency, Color color);

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

extern uint8_t blip_history[512];

// Brightness cursor, channel 1.
extern uint16_t blip_bright1;

// Brightness cursor, channel 2.
extern uint16_t blip_bright2;

//--------------------------------------------------------------------------------

#endif
