#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

#ifndef F_CPU
#define F_CPU 8000000
#endif

#define BOARDTYPE_PROTO3

// fuses for ATMega328p -
// low:      0xE2
// high:     0xDA
// extended: 0xFF

//-----------------------------------------------------------------------------
// board configs

/*
// prototype 2 settings
#define PORT_SOURCE PORTD
#define PORT_SINK DDRB
#define SINK_GREEN 0x0C
#define SINK_RED 0x12
#define SINK_BLUE 0x01;
*/

//-----------------------------------------------------------------------------
// prototype 3 settings
/*
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

#define SINK_RED   0x7D  // (~((1 << 1) | (1 << 7)))
#define SINK_GREEN 0xB3  // (~((1 << 2) | (1 << 3) | (1 << 6)))
#define SINK_BLUE  0xFE  // (~((1 << 0)))
*/

//-----------------------------------------------------------------------------
// test board from OSH Park

#ifdef BOARDTYPE_PROTO1
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

#define ADC_CHANNEL 0

#define SOURCE_1 0x02
#define SOURCE_2 0x04
#define SOURCE_3 0x01
#define SOURCE_4 0x08
#define SOURCE_5 0x10
#define SOURCE_6 0x40
#define SOURCE_7 0x20
#define SOURCE_8 0x80

#define SINK_RED   0xF1  // (~((1 << 1) | (1 << 7)))
#define SINK_GREEN 0x6E  // (~((1 << 2) | (1 << 3) | (1 << 6)))
#define SINK_BLUE  0x9F  // (~((1 << 0)))

#define BUTTON1_PIN 1
#define BUTTON2_PIN 1

#define MIC_POWER 7
#define MIC_PIN 0
#endif

//-----------------------------------------------------------------------------
// test board from OSH Park 2

#ifdef BOARDTYPE_PROTO2
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

#define ADC_CHANNEL 0

#define SOURCE_1 0x02
#define SOURCE_2 0x04
#define SOURCE_3 0x01
#define SOURCE_4 0x08
#define SOURCE_5 0x10
#define SOURCE_6 0x40
#define SOURCE_7 0x20
#define SOURCE_8 0x80

#define SINK_RED 0x6E
#define SINK_GREEN 0xF1
#define SINK_BLUE 0x9F

#define BUTTON1_PIN 1
#define BUTTON2_PIN 1

#define MIC_POWER 2
#define MIC_PIN 7
#endif

//-----------------------------------------------------------------------------
// test board from OSH Park 3

#ifdef BOARDTYPE_PROTO3
#define DIR_SOURCE  DDRD
#define DIR_SINK    DDRB
#define DIR_STATUS  DDRC

#define PORT_SOURCE PORTD
#define PORT_SINK   PORTB
#define PORT_STATUS PORTC

#define ADC_CHANNEL 0

#define LED_1 1
#define LED_2 

#define SOURCE_1 0x02
#define SOURCE_2 0x04
#define SOURCE_3 0x01
#define SOURCE_4 0x08
#define SOURCE_5 0x10
#define SOURCE_6 0x40
#define SOURCE_7 0x20
#define SOURCE_8 0x80

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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// audio processing configs

#define TRIG1_CLAMP  60
#define BRIGHT1_UP   (65535 / 30)
#define BRIGHT1_DOWN (65535 / 300)

#define TRIG2_CLAMP  60
#define BRIGHT2_UP   (65535 / 40)
#define BRIGHT2_DOWN (65535 / 700)

//-----------------------------------------------------------------------------

#endif /* CONFIG_H_ */