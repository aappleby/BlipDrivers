#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

#include <stdint.h>

void SetupLEDs();

extern uint8_t r[8];
extern uint8_t g[8];
extern uint8_t b[8];
void swap();
extern uint16_t led_tick; // 4.096 khz
extern volatile uint8_t blank;

#endif