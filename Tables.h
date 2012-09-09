/*
 * Tables.h
 *
 * Created: 8/9/2012 6:27:08 PM
 *  Author: aappleby
 */ 


#ifndef TABLES_H_
#define TABLES_H_

/*
 * Tables.cpp
 *
 * Created: 9/9/2012 3:34:29 PM
 *  Author: aappleby
 */ 

#include <stdint.h>
#include <avr/pgmspace.h>

uint8_t getSin(uint8_t x);
uint8_t getQuad(uint8_t x);
uint8_t getGammaSin(uint8_t x);
uint8_t getSparkle(uint8_t x);

#endif /* TABLES_H_ */