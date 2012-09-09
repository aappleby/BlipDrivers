/*
 * Config.h
 *
 * Created: 8/9/2012 5:57:22 PM
 *  Author: aappleby
 */ 

#ifndef CONFIG_H_
#define CONFIG_H_

#include "Defines.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "Tables.h"

typedef void(*Callback)();

extern Callback timer_callback;
extern Callback pattern_callback;

// Stand-alone board test functions

void testLEDs();

// Initialization

void SetupADC();
void SetupLEDs();

// from AudioProcessing.cpp

void UpdateAudio(int16_t sample);
void UpdateAudioSync();
extern uint8_t bright1;
extern uint8_t bright2;
extern uint16_t tmax1;
extern uint16_t tmax2;
extern uint8_t tickcount;

// from LEDDriver.cpp

extern uint8_t r[8];
extern uint8_t g[8];
extern uint8_t b[8];
void swap();
extern uint16_t led_tick; // 4.096 khz
extern volatile uint8_t blank;

// from Patterns.cpp

void RGBWaves();
void Gradients();
void StartupPattern();
void DumbPattern();
void Sparkles();
void Scroller();
void SpaceZoom();

#endif /* CONFIG_H_ */