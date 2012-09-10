/*
 * Tables.cpp
 *
 * Created: 9/9/2012 3:34:29 PM
 *  Author: aappleby
 */ 

#include "Tables.h"
#include <stdint.h>
#include <avr/pgmspace.h>

//-----------------------------------------------------------------------------

const uint8_t sintab[256] PROGMEM =
{	
128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173,
176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 
245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 
255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 
245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 
218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 
176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 
128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 
76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 
27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 
1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 
18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 
65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124
};

uint8_t getSin(uint8_t x) { return pgm_read_byte(sintab + x); }

const uint8_t quadtab[256] PROGMEM =
{
255, 251, 247, 243, 239, 235, 232, 228, 224, 220, 217, 213, 209, 206, 202, 199, 195, 192, 
188, 185, 181, 178, 175, 171, 168, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 
131, 128, 126, 123, 120, 117, 115, 112, 109, 107, 104, 102, 99, 97, 94, 92, 89, 87, 85, 82, 
80, 78, 76, 74, 71, 69, 67, 65, 63, 61, 59, 57, 56, 54, 52, 50, 48, 47, 45, 43, 42, 40, 38, 
37, 35, 34, 32, 31, 30, 28, 27, 26, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 
10, 9, 9, 8, 7, 7, 6, 5, 5, 4, 4, 3, 3, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 7, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 
17, 18, 19, 20, 21, 22, 23, 24, 26, 27, 28, 30, 31, 32, 34, 35, 37, 38, 40, 42, 43, 45, 47, 
48, 50, 52, 54, 56, 57, 59, 61, 63, 65, 67, 69, 71, 74, 76, 78, 80, 82, 85, 87, 89, 92, 94, 
97, 99, 102, 104, 107, 109, 112, 115, 117, 120, 123, 126, 128, 131, 134, 137, 140, 143, 146, 
149, 152, 155, 158, 162, 165, 168, 171, 175, 178, 181, 185, 188, 192, 195, 199, 202, 206, 209, 
213, 217, 220, 224, 228, 232, 235, 239, 243, 247, 251, 255,
};

uint8_t getQuad(uint8_t x) { return pgm_read_byte(quadtab + x); }

// Gamma-corrected sine wave
const uint8_t gammasin[] PROGMEM =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,2,2,2,2,3,3,4,4,5,5,6,7,8,9,9,10,
	11,13,14,15,16,18,19,21,23,24,26,28,30,32,34,37,39,41,44,46,49,52,55,58,61,
	64,67,70,73,77,80,84,87,91,95,98,102,106,110,114,118,122,126,130,134,138,142,146,
	150,154,158,162,166,170,174,178,182,186,190,193,197,200,204,207,211,214,217,220,
	223,226,228,231,234,236,238,240,242,244,246,247,249,250,251,252,253,254,254,255,
	255,255,255,255,254,254,253,252,251,250,249,247,246,244,242,240,238,236,234,231,
	228,226,223,220,217,214,211,207,204,200,197,193,190,186,182,178,174,170,166,162,
	158,154,150,146,142,138,134,130,126,122,118,114,110,106,102,98,95,91,87,84,80,77,
	73,70,67,64,61,58,55,52,49,46,44,41,39,37,34,32,30,28,26,24,23,21,19,18,16,15,14,
	13,11,10,9,9,8,7,6,5,5,4,4,3,3,2,2,2,2,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

uint8_t getGammaSin(uint8_t x) { return pgm_read_byte(gammasin + x); }

const uint8_t sparkles[256] PROGMEM =
{
	// Exponential spike, scale factor 1.25, 25 elements
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 20, 25, 31, 39, 48, 61, 76, 95, 119, 149, 186, 232, 186, 149, 119, 95, 76, 61, 48, 39, 31, 25, 20, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

uint8_t getSparkle(uint8_t x) { return pgm_read_byte(sparkles + x); }