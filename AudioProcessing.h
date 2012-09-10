/*
 * AudioProcessing.h
 *
 * Created: 9/9/2012 3:23:35 PM
 *  Author: aappleby
 */ 


#ifndef AUDIOPROCESSING_H_
#define AUDIOPROCESSING_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void UpdateAudioSync();
extern uint8_t bright1;
extern uint8_t bright2;
extern uint16_t tmax1;
extern uint16_t tmax2;
extern uint8_t tickcount;

#ifdef __cplusplus
}
#endif

#endif /* AUDIOPROCESSING_H_ */