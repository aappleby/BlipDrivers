#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void SetupLEDs();

extern uint8_t r[8];
extern uint8_t g[8];
extern uint8_t b[8];
void swap();

extern volatile uint8_t blank;
extern uint16_t led_tick; // 4.096 khz

extern uint8_t bright1;
extern uint8_t bright2;
extern uint16_t tmax1;
extern uint16_t tmax2;

#ifdef __cplusplus
}
#endif

#endif