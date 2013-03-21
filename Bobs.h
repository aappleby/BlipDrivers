/*
 * Bobs.h
 *
 * Created: 3/8/2013 8:58:10 PM
 *  Author: aappleby
 */ 


#ifndef BOBS_H_
#define BOBS_H_

#include <stdint.h>

uint8_t add_smooth(uint8_t a, uint8_t b);

class Bob
{
public:

	void render();

    uint16_t speed;
	uint16_t phase;
	uint32_t timestamp;
};



#endif /* BOBS_H_ */