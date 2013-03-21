#include <util/delay.h>
#include "LEDDriver.h"

void setup() {
  SetupLEDs();
}

// Gamma-corrected sine wave
const uint8_t gammasin[] PROGMEM =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,2,2,2,2,3,3,4,4,5,5,6,7,8,9,9,10,
	11,13,14,15,16,18,19,21,23,24,26,28,30,32,34,37,39,41,44,46,49,52,55,58,61,
	64,67,70,73,77,80,84,87,91,95,98,102,106,110,114,118,122,126,130,134,138,142,146,
	150,154,158,162,166,170,174,178,182,186,190,193,197,200,204,207,211,214,217,220,
	223,226,228,231,234,236,238,240,242,244,246,247,249,250,251,252,253,254,254,255,
	255,255,255,255,254,254,253,252,251,250,249,247,246,244,242,240,238,236,234,231,
	228,226,223,220,217,214,211,207,204,200,197,193,190,186,182,178,174,170,166,162,
	158,154,150,146,142,138,134,130,126,122,118,114,110,106,102,98,95,91,87,84,80,77,
	73,70,67,64,61,58,55,52,49,46,44,41,39,37,34,32,30,28,26,24,23,21,19,18,16,15,14,
	13,11,10,9,9,8,7,6,5,5,4,4,3,3,2,2,2,2,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

void FastWaves() {
	static uint16_t timerR;
	static uint16_t timerG;
	static uint16_t timerB;
	
	const int stepR = 25;
	const int stepG = 26;
	const int stepB = 27;
	const int speedG = -5;
	const int speedR = -6;
	const int speedB = -7;
	
	
	timerR += speedR;
	timerG += speedG;
	timerB += speedB;
	uint8_t phaseR = timerR >> 8;
	uint8_t phaseG = timerG >> 8;
	uint8_t phaseB = timerB >> 8;
	
	uint8_t cursorR = phaseR;
	uint8_t cursorG = phaseG;
	uint8_t cursorB = phaseB;
	for(int i = 0; i < 8; i++) {
		r[i] = pgm_read_byte(gammasin + cursorR);
		g[i] = pgm_read_byte(gammasin + cursorG);
		b[i] = pgm_read_byte(gammasin + cursorB);
		
		cursorR += stepR;
		cursorG += stepG;
		cursorB += stepB;
	}		
}	

#define test 8

void loop() {
  //FastWaves();
  for(int i = 0; i < 100; i++) {
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 1;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 1;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 1;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 1;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 1;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 1;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 1;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 1;
    swap();
  }
  for(int i = 0; i < 100; i++) {
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 8;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 0;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 0;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 0;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 0;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 0;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 0;
    swap();
    r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 0;
  }
}

