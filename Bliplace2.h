#ifndef BLIPLACE_H_
#define BLIPLACE_H_

#include <stdint.h>

#define BOARDTYPE_PROTO3
//#define BOARDTYPE_MICRO

//--------------------------------------------------------------------------------
// Our "framebuffer" is a simple array of 8 RGB pixels.

struct Color {
  uint16_t r;
  uint16_t g;
  uint16_t b;
};

extern Color blip_pixels[8];

//--------------------------------------------------------------------------------

// Our global 'clock' ticks 4096 times a second.
extern volatile uint32_t blip_tick;

// Button debounce counters
extern volatile uint8_t buttonstate1;
extern volatile uint16_t debounce_up1;
extern volatile uint16_t debounce_down1;

extern volatile uint8_t buttonstate2;
extern volatile uint16_t debounce_up2;
extern volatile uint16_t debounce_down2;

// Brightness cursor, channel 1.
extern uint16_t blip_bright1;

// Brightness cursor, channel 2.
extern uint16_t blip_bright2;

//--------------------------------------------------------------------------------

// Install the Bliplace LED drivers & interrupts.
void blip_setup();

// Turn off all peripherals, disable all interrupts.
void blip_shutdown();

// Put the Bliplace to sleep - it will wake up (and return from this function
// call) when the button has been pressed for 1/8 second.
void blip_sleep();

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

uint16_t blip_time();
uint16_t blip_tempo(uint16_t bpm);

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

Color    blip_color(uint16_t r, uint16_t g, uint16_t b);
Color    blip_color(uint16_t hue);
Color    blip_color(const char* hexcode);

Color    blip_scale(Color const& c, uint16_t s);
Color    blip_scale(Color const& c, uint16_t s1, uint16_t s2);
Color    blip_add(Color const& a, Color const& b);
Color    blip_smadd(Color const& a, Color const& b);
Color    blip_lerp(Color const& a, Color const& b, uint16_t x);

//--------------------------------------------------------------------------------

void blip_draw_sin(uint16_t phase, uint16_t frequency, Color color);

//--------------------------------------------------------------------------------
// Board configuration.

// fuses for ATMega328p -
// low:      0xE2
// high:     0xDA
// extended: 0xFF

// prototype 2 settings
/*
#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define SINK_GREEN 0x0C
#define SINK_RED 0x12
#define SINK_BLUE 0x01;
*/

// prototype 3 settings
/*
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

#define SINK_RED   0x7D  // (~((1 << 1) | (1 << 7)))
#define SINK_GREEN 0xB3  // (~((1 << 2) | (1 << 3) | (1 << 6)))
#define SINK_BLUE  0xFE  // (~((1 << 0)))
*/

//--------------------------------------------------------------------------------
// test board from OSH Park

#ifdef BOARDTYPE_PROTO1
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

enum {
  PIXEL_0_TO_PIN = 1, PIXEL_1_TO_PIN = 2, PIXEL_2_TO_PIN = 0, PIXEL_3_TO_PIN = 3,
  PIXEL_4_TO_PIN = 4, PIXEL_5_TO_PIN = 6, PIXEL_6_TO_PIN = 5, PIXEL_7_TO_PIN = 7,
  
  PIN_0_TO_PIXEL = 2, PIN_1_TO_PIXEL = 0, PIN_2_TO_PIXEL = 1, PIN_3_TO_PIXEL = 3,
  PIN_4_TO_PIXEL = 4, PIN_5_TO_PIXEL = 6, PIN_6_TO_PIXEL = 5, PIN_7_TO_PIXEL = 7,
};  

#define SINK_RED   0xF1  // (~((1 << 1) | (1 << 7)))
#define SINK_GREEN 0x6E  // (~((1 << 2) | (1 << 3) | (1 << 6)))
#define SINK_BLUE  0x9F  // (~((1 << 0)))

#define BUTTON1_PIN 1
#define BUTTON2_PIN 1

#define MIC_POWER 7
#define MIC_PIN 0
#endif

//--------------------------------------------------------------------------------
// test board from OSH Park 2

#ifdef BOARDTYPE_PROTO2
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

enum {
  PIXEL_0_TO_PIN = 1, PIXEL_1_TO_PIN = 2, PIXEL_2_TO_PIN = 0, PIXEL_3_TO_PIN = 3,
  PIXEL_4_TO_PIN = 4, PIXEL_5_TO_PIN = 6, PIXEL_6_TO_PIN = 5, PIXEL_7_TO_PIN = 7,
  
  PIN_0_TO_PIXEL = 2, PIN_1_TO_PIXEL = 0, PIN_2_TO_PIXEL = 1, PIN_3_TO_PIXEL = 3,
  PIN_4_TO_PIXEL = 4, PIN_5_TO_PIXEL = 6, PIN_6_TO_PIXEL = 5, PIN_7_TO_PIXEL = 7,
};  

#define SINK_RED 0x6E
#define SINK_GREEN 0xF1
#define SINK_BLUE 0x9F

#define BUTTON1_PIN 1
#define BUTTON2_PIN 1

#define MIC_POWER 2
#define MIC_PIN 7
#endif

//--------------------------------------------------------------------------------
// test board from OSH Park 3

#ifdef BOARDTYPE_PROTO3
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

enum {
  PIXEL_0_TO_PIN = 1, PIXEL_1_TO_PIN = 2, PIXEL_2_TO_PIN = 0, PIXEL_3_TO_PIN = 3,
  PIXEL_4_TO_PIN = 4, PIXEL_5_TO_PIN = 6, PIXEL_6_TO_PIN = 5, PIXEL_7_TO_PIN = 7,
  
  PIN_0_TO_PIXEL = 2, PIN_1_TO_PIXEL = 0, PIN_2_TO_PIXEL = 1, PIN_3_TO_PIXEL = 3,
  PIN_4_TO_PIXEL = 4, PIN_5_TO_PIXEL = 6, PIN_6_TO_PIXEL = 5, PIN_7_TO_PIXEL = 7,
};  

#define SINK_RED 0x6E
#define SINK_GREEN 0xF1
#define SINK_BLUE 0x9F

#define BUTTON1_PIN 3
#define BUTTON2_PIN 4

#define MIC_POWER 2
#define MIC_PIN 7
#endif

//--------------------------------------------------------------------------------
// breadboard prototype w/ usb

#ifdef BOARDTYPE_BREADBOARD
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

enum {
  PIXEL_0_TO_PIN = 0, PIXEL_1_TO_PIN = 1, PIXEL_2_TO_PIN = 2, PIXEL_3_TO_PIN = 3,
  PIXEL_4_TO_PIN = 4, PIXEL_5_TO_PIN = 5, PIXEL_6_TO_PIN = 6, PIXEL_7_TO_PIN = 7,
  
  PIN_0_TO_PIXEL = 0, PIN_1_TO_PIXEL = 1, PIN_2_TO_PIXEL = 2, PIN_3_TO_PIXEL = 3,
  PIN_4_TO_PIXEL = 4, PIN_5_TO_PIXEL = 5, PIN_6_TO_PIXEL = 6, PIN_7_TO_PIXEL = 7,
};  

#define SINK_RED 0x6E
#define SINK_GREEN 0xF1
#define SINK_BLUE 0x9F

#define BUTTON1_PIN 2
#define BUTTON2_PIN 2

#define MIC_POWER 0
#define MIC_PIN 1

#define USING_CRYSTAL

#endif

//--------------------------------------------------------------------------------
// POVLace 1

#ifdef BOARDTYPE_POVLACE1
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

#define SOURCE_1 0x02
#define SOURCE_2 0x04
#define SOURCE_3 0x01
#define SOURCE_4 0x08
#define SOURCE_5 0x10
#define SOURCE_6 0x40
#define SOURCE_7 0x20
#define SOURCE_8 0x80

#define SINK_RED   0xDE
#define SINK_GREEN 0xED
#define SINK_BLUE  0xF3

#define MIC_PIN 7
#define MIC_POWER 2
#define BUTTON1_PIN 3
#define BUTTON2_PIN 3
#endif

//--------------------------------------------------------------------------------
// Teensy 2.0, ATMega32u4 w/ 16 mhz crystal.

#ifdef BOARDTYPE_TEENSY2
#define DIR_SOURCE  DDRB
#define DIR_SINK    DDRD
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTB
#define PORT_SINK   PORTD
#define PORT_STATUS PORTC

enum {
  PIXEL_0_TO_PIN = 0, PIXEL_1_TO_PIN = 1, PIXEL_2_TO_PIN = 2, PIXEL_3_TO_PIN = 3,
  PIXEL_4_TO_PIN = 4, PIXEL_5_TO_PIN = 5, PIXEL_6_TO_PIN = 6, PIXEL_7_TO_PIN = 7,
  
  PIN_0_TO_PIXEL = 0, PIN_1_TO_PIXEL = 1, PIN_2_TO_PIXEL = 2, PIN_3_TO_PIXEL = 3,
  PIN_4_TO_PIXEL = 4, PIN_5_TO_PIXEL = 5, PIN_6_TO_PIXEL = 6, PIN_7_TO_PIXEL = 7,
};  

#define SINK_RED 0xFE
#define SINK_GREEN 0xFD
#define SINK_BLUE 0xFB

#define BUTTON1_PIN 1
#define BUTTON2_PIN 1

#define MIC_POWER 2
#define MIC_PIN 7

#define USING_CRYSTAL

#endif

//--------------------------------------------------------------------------------

#ifdef BOARDTYPE_MICRO
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

enum {
	PIXEL_0_TO_PIN = 0, PIXEL_1_TO_PIN = 1, PIXEL_2_TO_PIN = 2, PIXEL_3_TO_PIN = 3,
	PIXEL_4_TO_PIN = 4, PIXEL_5_TO_PIN = 5, PIXEL_6_TO_PIN = 6, PIXEL_7_TO_PIN = 7,
	
	PIN_0_TO_PIXEL = 0, PIN_1_TO_PIXEL = 1, PIN_2_TO_PIXEL = 2, PIN_3_TO_PIXEL = 3,
	PIN_4_TO_PIXEL = 4, PIN_5_TO_PIXEL = 5, PIN_6_TO_PIXEL = 6, PIN_7_TO_PIXEL = 7,
};

#define SINK_RED 0xFD
#define SINK_GREEN 0xFE
#define SINK_BLUE 0xFB

#define BUTTON1_PIN 1
#define BUTTON2_PIN 1

#define MIC_POWER 2
#define MIC_PIN 1

#endif

//--------------------------------------------------------------------------------

#endif  // #ifndef BLIPLACE_H_
