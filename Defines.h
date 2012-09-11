#ifndef DEFINES_H_
#define DEFINES_H_

//-----------------------------------------------------------------------------
// generic macros

#define bit(A)   (1 << A)
#define sbi(p,b) { p |= (unsigned char)bit(b); }
#define cbi(p,b) { p &= (unsigned char)~bit(b); }
#define tbi(p,b) { p ^= (unsigned char)bit(b); }
#define gbi(p,b) (p & (unsigned char)bit(b))

#define lo8(A) (((uint16_t)A) & 0xFF)
#define hi8(A) (((uint16_t)A) >> 8)

//-----------------------------------------------------------------------------
// board configs

#define F_CPU 8000000

/*
// prototype 2 settings
#define PORT_SOURCE PORTD
#define PORT_SINK DDRB
#define SINK_GREEN 0x0C
#define SINK_RED 0x12
#define SINK_BLUE 0x01;
*/

// prototype 3 settings
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
#define SOURCE_5 0x40
#define SOURCE_6 0x10
#define SOURCE_7 0x20
#define SOURCE_8 0x80

#define ADC_CHANNEL 1

#define SINK_RED 0x7D   // (~((1 << 1) | (1 << 7)))
#define SINK_GREEN 0xB3 // (~((1 << 2) | (1 << 3) | (1 << 6)))
#define SINK_BLUE 0xFE  // (~((1 << 0)))

//-----------------------------------------------------------------------------
// audio processing configs

#define TRIG1_CLAMP  60
#define BRIGHT1_UP   (65535 / 30)
#define BRIGHT1_DOWN (65535 / 300)

#define TRIG2_CLAMP  60
#define BRIGHT2_UP   (65535 / 40)
#define BRIGHT2_DOWN (65535 / 700)

//-----------------------------------------------------------------------------

#endif