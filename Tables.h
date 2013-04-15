#ifndef TABLES_H_
#define TABLES_H_

#include <avr/pgmspace.h>

// Various lookup tables for patterns & the LED driver.

extern const uint8_t exptab[256] PROGMEM;
extern const uint8_t gammatab[256] PROGMEM;
extern const uint8_t cielum[256] PROGMEM;

extern const uint8_t sintab[256] PROGMEM;
extern const int8_t  ssintab[256] PROGMEM;

extern const uint8_t quadtab[256] PROGMEM;
extern const uint8_t gammapulse[256] PROGMEM;
extern const uint8_t pulse_5_2[256] PROGMEM;
extern const uint8_t pulse_5_1[256] PROGMEM;
extern const uint8_t pulse_5_3[256] PROGMEM;
extern const uint8_t pulse_5_4[256] PROGMEM;
extern const uint8_t pulse_5_6[256] PROGMEM;
extern const uint8_t pulse_2_2[256] PROGMEM;
extern const uint8_t pulse_2_3[256] PROGMEM;
extern const uint8_t pulse_2_4[256] PROGMEM;
extern const uint8_t pulse_2_6[256] PROGMEM;
extern const uint8_t gamma_2_2[256] PROGMEM;

extern const uint8_t noise[256] PROGMEM;
extern const uint8_t huetab[256] PROGMEM;

#endif