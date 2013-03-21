/*
 * Bobs.cpp
 *
 * Created: 3/8/2013 8:56:40 PM
 *  Author: aappleby
 */ 

#include "Bobs.h"

#include "LEDDriver.h"

/*
uint8_t add_smooth(uint8_t a, uint8_t b) {
	return (((a + b) << 8) - (a*b)) >> 8;
}
*/

__attribute__((naked)) uint8_t add_smooth( uint8_t a, uint8_t b) {
	asm("mul r22, r24");
	asm("add r24, r22");
	asm("clr r22");
	asm("sub r22, r0");
	asm("sbc r24, r1");
	asm("clr r1");
	asm("ret");
};


void Bob::render(void)
{
		
}