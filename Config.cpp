#include "Config.h"

Callback timer_callback;
Callback pattern_callback;

//---------------------------------------------------------------------------
// Set up ADC. Default config assumes F_CPU == 8000000
// We sample at 1000000 / (16*14) = ~4464 hz

#if F_CPU == 8000000
void SetupADC() {
	// conversion takes 28 uS
	ADMUX  = bit(MUX0) | bit(ADLAR);
	ADCSRA = bit(ADEN) | bit(ADPS2);
	sbi(ADCSRA,ADSC);
}	
#endif
