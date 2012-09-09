#ifndef DEFINES_H_
#define DEFINES_H_

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
#define DIR_SOURCE DDRD

#define PORT_SOURCE PORTD
#define PORT_SINK DDRB
#define PORT_STATUS PORTC

//#define PORT_SINK PORTB
//#define DIR_SINK DDRB
#define SOURCE_1 0x02
#define SOURCE_2 0x04
#define SOURCE_3 0x01
#define SOURCE_4 0x08
#define SOURCE_5 0x40
#define SOURCE_6 0x10
#define SOURCE_7 0x20
#define SOURCE_8 0x80
#define SINK_RED ((1 << 1) | (1 << 7))
#define SINK_GREEN ((1 << 2) | (1 << 3) | (1 << 6))
#define SINK_BLUE ((1 << 0))


#define bit(A)   (1 << A)
#define sbi(p,b) { p |= (unsigned char)bit(b); }
#define cbi(p,b) { p &= (unsigned char)~bit(b); }
#define tbi(p,b) { p ^= (unsigned char)bit(b); }
#define gbi(p,b) (p & (unsigned char)bit(b))

#endif