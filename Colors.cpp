#include "Colors.h"
#include "Tables.h"

#include <avr/pgmspace.h>

__attribute__((naked)) uint8_t add_smooth( uint8_t a, uint8_t b) {
	asm("mul r22, r24");
	asm("add r24, r22");
	asm("clr r22");
	asm("sub r22, r0");
	asm("sbc r24, r1");
	asm("clr r1");
	asm("ret");
};

inline uint8_t crossfade (uint8_t a, uint8_t b, uint8_t alpha) {
  return (a*alpha + b*(255-alpha)) >> 8;
}



__attribute__((naked)) uint16_t fixmul2 ( uint16_t a, uint16_t b)
{
	// a = r25:r24
	// b = r23:r22
	// return = r25:r24
	// temp: r27:r26
	
	asm("clr r26");
	asm("clr r27");
	
	// ah * bl
	asm("mul r25,r22");
	asm("movw r26, r0");
	
	// al * bh
	asm("mul r24, r23");
	asm("add r26, r0");
	asm("adc r27, r1");
	
	// ah * bh
	asm("mul r25, r23");
	asm("add r27, r0");
	
	// al * bl & clear r1
	asm("mul r24, r22");
	asm("add r26, r1");
	asm("clr r1");
	asm("adc r27, r1");

	// done
	asm("movw r24, r26");
	asm("ret");
}

__attribute__((noinline)) uint16_t fixmul ( uint16_t a, uint16_t b)
{
	uint8_t ah = a >> 8;
	uint8_t al = a;
	uint8_t bh = b >> 8;
	uint8_t bl = b;
	
	uint16_t out1 = (al * bl) >> 8;
	uint16_t out2 = (al * bh);
	uint16_t out3 = (ah * bl);
	uint16_t out4 = (ah * bh) << 8;
	
	return out1 + out2 + out3 + out4;
}	
