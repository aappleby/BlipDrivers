// asm driver

// Code here -only- touches R30 and R31 to minimize register save/load
// overhead

#include "Defines.h"

#include <avr/io.h>

#define TIMER_LO _SFR_IO_ADDR(TCNT1L)
#define TIMER_HI _SFR_IO_ADDR(TCNT1H)

#define CALLBACK_LO timer_callback
#define CALLBACK_HI timer_callback+1

#define INTERRUPT_OVERHEAD 18

.global bits_red_4, bits_red_5, bits_red_6, bits_red_7
.global bits_green_4, bits_green_5, bits_green_6, bits_green_7
.global bits_blue_4, bits_blue_5, bits_blue_6, bits_blue_7
.global TIMER1_OVF_vect
.extern timer_callback, blank

//------------------------------------------------------------------------------

bits_blue_4a:
// clear old source
CLR R30
OUT _SFR_IO_ADDR(PORT_SOURCE), R30

// switch sinks
LDI R30, SINK_BLUE
OUT _SFR_IO_ADDR(PORT_SINK), R30

// send pulse
LDS R30, bits_BF + 4
OUT _SFR_IO_ADDR(PORT_SOURCE), R30

// set next timeout
LDI R30, lo8(65536 - 48 + INTERRUPT_OVERHEAD)
LDI R31, hi8(65536 - 48 + INTERRUPT_OVERHEAD)
STS TCNT1H,R31
STS TCNT1L,R30

// set next callback
LDI R30, pm_lo8(bits_blue_5)
LDI R31, pm_hi8(bits_blue_5)
STS CALLBACK_HI,R31
STS CALLBACK_LO,R30
RET

//------------------------------------------------------------------------------
