/*
 * Colors.h
 *
 * Created: 3/13/2013 11:22:13 PM
 *  Author: aappleby
 */ 


#ifndef COLORS_H_
#define COLORS_H_

#include <stdint.h>

void hue_to_rgb(uint8_t hue, uint8_t& r, uint8_t& g, uint8_t& b);

void hue_to_rgb2(uint32_t hue, uint8_t& r, uint8_t& g, uint8_t& b);




#endif /* COLORS_H_ */