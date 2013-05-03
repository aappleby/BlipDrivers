#ifndef BOBS_H_
#define BOBS_H_

#include <stdint.h>

class Bob
{
public:

	void render();

  uint16_t speed;
	uint16_t phase;
	uint32_t timestamp;
};



#endif