#include <avr/pgmspace.h>

//-----------------------------------------------------------------------------
// Exponential ramp.

extern const uint8_t exptab[256] PROGMEM =
{
  0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,5,5,
  5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9,9,10,10,10,
  10,10,11,11,11,11,12,12,12,12,13,13,13,14,14,14,14,15,15,15,16,16,16,17,
  17,18,18,18,19,19,20,20,21,21,21,22,22,23,23,24,24,25,25,26,27,27,28,28,
  29,30,30,31,32,32,33,34,35,35,36,37,38,39,39,40,41,42,43,44,45,46,47,48,
  49,50,51,52,53,55,56,57,58,59,61,62,63,65,66,68,69,71,72,74,76,77,79,81,
  82,84,86,88,90,92,94,96,98,100,102,105,107,109,112,114,117,119,122,124,
  127,130,133,136,139,142,145,148,151,155,158,162,165,169,172,176,180,184,
  188,192,196,201,205,210,214,219,224,229,234,239,244,250,255,
};

// Numeric brightness -> perceived brightness

extern const uint8_t gammatab[256] PROGMEM =
{
  0,15,22,27,31,35,39,42,45,47,50,52,55,57,59,61,63,65,67,69,71,73,74,76,78,79,
  81,82,84,85,87,88,90,91,93,94,95,97,98,99,100,102,103,104,105,107,108,109,110,
  111,112,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,
  131,132,133,134,135,136,137,138,139,140,141,141,142,143,144,145,146,147,148,
  148,149,150,151,152,153,153,154,155,156,157,158,158,159,160,161,162,162,163,
  164,165,165,166,167,168,168,169,170,171,171,172,173,174,174,175,176,177,177,
  178,179,179,180,181,182,182,183,184,184,185,186,186,187,188,188,189,190,190,
  191,192,192,193,194,194,195,196,196,197,198,198,199,200,200,201,201,202,203,
  203,204,205,205,206,206,207,208,208,209,210,210,211,211,212,213,213,214,214,
  215,216,216,217,217,218,218,219,220,220,221,221,222,222,223,224,224,225,225,
  226,226,227,228,228,229,229,230,230,231,231,232,233,233,234,234,235,235,236,
  236,237,237,238,238,239,240,240,241,241,242,242,243,243,244,244,245,245,246,
  246,247,247,248,248,249,249,250,250,251,251,252,252,253,253,254,255,
};


//-----------------------------------------------------------------------------
// CIE 1931 luminosity

extern const uint8_t cielum[] PROGMEM =
{
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,
  1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,
  3,   3,   4,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,   6,   6,   6,
  6,   7,   7,   7,   7,   8,   8,   8,   8,   9,   9,   9,   10,  10,  10,  11,
  11,  11,  12,  12,  12,  13,  13,  13,  14,  14,  14,  15,  15,  16,  16,  16,
  17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  23,  24,  24,
  25,  25,  26,  26,  27,  28,  28,  29,  29,  30,  31,  31,  32,  33,  33,  34,
  35,  35,  36,  37,  37,  38,  39,  40,  40,  41,  42,  43,  44,  44,  45,  46,
  47,  48,  49,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,
  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,
  79,  80,  82,  83,  84,  85,  87,  88,  89,  90,  92,  93,  94,  96,  97,  99,
  100, 101, 103, 104, 106, 107, 108, 110, 111, 113, 114, 116, 118, 119, 121, 122,
  124, 125, 127, 129, 130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 148, 149,
  151, 153, 155, 157, 159, 161, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180,
  182, 185, 187, 189, 191, 193, 195, 197, 200, 202, 204, 206, 208, 211, 213, 215,
  218, 220, 222, 225, 227, 230, 232, 234, 237, 239, 242, 244, 247, 249, 252, 255,
};

extern const uint8_t ciesin[256] PROGMEM =
{	
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   3,   3,   3,   3,   4,
	4,   5,   5,   6,   6,   7,   7,   8,   9,   9,   10,  11,  12,  13,  14,  15,
	16,  18,  19,  20,  22,  23,  25,  27,  29,  30,  32,  35,  37,  39,  41,  44,
	46,  49,  52,  55,  58,  61,  64,  67,  71,  74,  77,  81,  85,  89,  92,  96,
	100, 104, 108, 113, 117, 121, 125, 130, 134, 139, 143, 148, 152, 156, 161, 165,
	170, 174, 178, 183, 187, 191, 195, 199, 203, 207, 210, 214, 218, 221, 224, 227,
	230, 233, 236, 238, 241, 243, 245, 247, 248, 250, 251, 252, 253, 254, 254, 254,
	255, 254, 254, 254, 253, 252, 251, 250, 248, 247, 245, 243, 241, 238, 236, 233,
	230, 227, 224, 221, 218, 214, 210, 207, 203, 199, 195, 191, 187, 183, 178, 174,
	170, 165, 161, 156, 152, 148, 143, 139, 134, 130, 125, 121, 117, 113, 108, 104,
	100, 96,  92,  89,  85,  81,  77,  74,  71,  67,  64,  61,  58,  55,  52,  49,
	46,  44,  41,  39,  37,  35,  32,  30,  29,  27,  25,  23,  22,  20,  19,  18,
	16,  15,  14,  13,  12,  11,  10,  9,   9,   8,   7,   7,   6,   6,   5,   5,
	4,   4,   3,   3,   3,   3,   2,   2,   2,   2,   2,   1,   1,   1,   1,   1,
	1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

extern const uint8_t sintab[256] PROGMEM =
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

extern const uint8_t quadtab[256] PROGMEM =
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

// Gamma-corrected sine wave
extern const uint8_t gammasin[] PROGMEM =
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

extern const uint8_t gammapulse[256] PROGMEM =
{
	//0,0,0,0,0,0,0,0,0,0,0,1,1,2,3,4,
	//5,6,8,10,12,15,17,20,24,28,32,36,41,46,51,57,
	//63,70,76,83,91,98,106,113,121,129,138,146,154,162,170,178,
	//185,193,200,207,213,220,225,231,235,240,244,247,250,252,253,254,
	0, 255,254,253,252,250,247,244,240,235,231,225,220,213,207,200,193,
	185,178,170,162,154,146,138,129,121,113,106,98,91,83,76,70,
	63,57,51,46,41,36,32,28,24,20,17,15,12,10,8,6,
	5,4,3,2,1,1,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

extern const uint8_t sparkles[256] PROGMEM =
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

extern const uint8_t pixel16[256] PROGMEM =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255,

	255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

extern const uint8_t pulse_5_2[256] PROGMEM =
{
0,189,220,236,245,250,253,255,255,255,254,252,251,249,246,244,
241,239,236,233,230,228,225,222,219,216,213,210,207,204,202,199,
196,193,190,188,185,182,180,177,175,172,169,167,164,162,160,157,
155,153,150,148,146,144,141,139,137,135,133,131,129,127,125,123,
121,119,117,116,114,112,110,109,107,105,103,102,100,99,97,95,
94,92,91,89,88,86,85,84,82,81,80,78,77,76,74,73,
72,71,69,68,67,66,65,64,63,61,60,59,58,57,56,55,
54,53,52,51,50,49,48,48,47,46,45,44,43,42,42,41,
40,39,38,38,37,36,35,35,34,33,33,32,31,31,30,29,
29,28,27,27,26,26,25,24,24,23,23,22,22,21,21,20,
20,19,19,18,18,17,17,17,16,16,15,15,14,14,14,13,
13,13,12,12,11,11,11,10,10,10,10,9,9,9,8,8,
8,8,7,7,7,7,6,6,6,6,5,5,5,5,5,4,
4,4,4,4,3,3,3,3,3,3,3,2,2,2,2,2,
2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

extern const uint8_t pulse_5_1[256] PROGMEM =
{
0,219,237,245,250,252,254,255,255,255,254,254,253,252,251,249,
248,247,245,244,242,241,239,238,236,235,233,232,230,228,227,225,
224,222,220,219,217,216,214,213,211,209,208,206,205,203,202,200,
199,197,196,194,193,191,190,188,187,186,184,183,181,180,179,177,
176,174,173,172,170,169,168,166,165,164,162,161,160,159,157,156,
155,153,152,151,150,149,147,146,145,144,142,141,140,139,138,137,
135,134,133,132,131,130,128,127,126,125,124,123,122,121,120,119,
117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,
101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,
85,84,84,83,82,81,80,79,78,77,76,75,74,74,73,72,
71,70,69,68,67,67,66,65,64,63,62,61,61,60,59,58,
57,57,56,55,54,53,52,52,51,50,49,48,48,47,46,45,
45,44,43,42,41,41,40,39,38,38,37,36,35,35,34,33,
32,32,31,30,30,29,28,27,27,26,25,25,24,23,22,22,
21,20,20,19,18,18,17,16,16,15,14,14,13,12,12,11,
10,10,9,8,8,7,6,6,5,4,4,3,3,2,1,1,	
};

extern const uint8_t pulse_5_3[256] PROGMEM = 
{
0,163,204,226,240,247,252,254,255,254,253,251,248,245,242,238,
235,231,227,223,219,215,211,207,203,199,195,191,187,183,179,176,
172,168,165,161,158,154,151,148,144,141,138,135,132,129,126,124,
121,118,115,113,110,108,105,103,101,98,96,94,92,90,88,86,
84,82,80,78,76,74,73,71,69,68,66,64,63,61,60,58,
57,56,54,53,52,50,49,48,47,46,44,43,42,41,40,39,
38,37,36,35,34,33,33,32,31,30,29,29,28,27,26,26,
25,24,24,23,22,22,21,21,20,19,19,18,18,17,17,16,
16,15,15,14,14,14,13,13,12,12,12,11,11,11,10,10,
10,9,9,9,8,8,8,8,7,7,7,7,6,6,6,6,
5,5,5,5,5,5,4,4,4,4,4,4,3,3,3,3,
3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	
};

extern const uint8_t pulse_5_4[256] PROGMEM =
{
	0,140,190,218,235,245,251,254,255,254,252,250,246,242,238,233,
	228,223,218,213,208,203,198,193,188,183,178,173,169,164,159,155,
	151,146,142,138,134,130,127,123,119,116,113,109,106,103,100,97,
	94,91,89,86,83,81,78,76,74,72,69,67,65,63,61,59,
	58,56,54,52,51,49,48,46,45,43,42,41,39,38,37,36,
	35,33,32,31,30,29,28,27,27,26,25,24,23,22,22,21,
	20,20,19,18,18,17,16,16,15,15,14,14,13,13,12,12,
	11,11,11,10,10,10,9,9,9,8,8,8,7,7,7,6,
	6,6,6,6,5,5,5,5,5,4,4,4,4,4,3,3,
	3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,
	2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

extern const uint8_t pulse_5_6[256] PROGMEM =
{
	0,104,163,201,225,240,249,254,255,254,251,247,242,236,230,223,
	216,209,202,195,188,181,174,168,161,155,149,143,137,131,126,121,
	116,111,106,102,97,93,89,85,82,78,75,72,68,65,63,60,
	57,55,52,50,48,46,44,42,40,38,36,35,33,32,30,29,
	27,26,25,24,23,22,21,20,19,18,17,16,15,15,14,13,
	13,12,12,11,10,10,9,9,9,8,8,7,7,7,6,6,
	6,5,5,5,5,4,4,4,4,4,3,3,3,3,3,3,
	2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

extern const uint8_t pulse_2_2[256] PROGMEM =
{
	0,10,19,28,37,46,55,63,71,79,86,94,101,108,115,121,
	127,134,140,145,151,156,162,167,172,176,181,185,189,193,197,201,
	205,208,211,215,218,221,223,226,228,231,233,235,237,239,241,242,
	244,245,247,248,249,250,251,252,252,253,254,254,254,255,255,255,
	255,255,255,255,254,254,254,253,253,252,251,251,250,249,248,247,
	246,245,244,243,242,241,239,238,237,235,234,232,231,229,228,226,
	225,223,221,219,218,216,214,212,210,209,207,205,203,201,199,197,
	195,193,191,189,187,185,183,180,178,176,174,172,170,168,166,164,
	161,159,157,155,153,151,149,147,144,142,140,138,136,134,132,130,
	128,125,123,121,119,117,115,113,111,109,107,105,103,101,99,97,
	95,93,92,90,88,86,84,82,80,79,77,75,73,72,70,68,
	67,65,63,62,60,59,57,56,54,52,51,50,48,47,45,44,
	43,41,40,39,37,36,35,34,32,31,30,29,28,27,26,25,
	24,23,22,21,20,19,18,17,16,16,15,14,13,12,12,11,
	10,10,9,8,8,7,7,6,6,5,5,4,4,4,3,3,
	3,2,2,2,1,1,1,1,1,0,0,0,0,0,0,0,
};

extern const uint8_t pulse_2_3[256] PROGMEM =
{
	0,2,5,9,14,20,25,31,37,44,50,57,63,70,77,83,
	90,97,103,110,116,123,129,135,141,147,152,158,163,169,174,179,
	183,188,193,197,201,205,209,213,216,219,223,226,229,231,234,236,
	238,241,243,244,246,247,249,250,251,252,253,254,254,254,255,255,
	255,255,255,254,254,254,253,252,252,251,250,249,247,246,245,244,
	242,241,239,237,236,234,232,230,228,226,224,222,220,218,215,213,
	211,208,206,204,201,199,196,194,191,189,186,183,181,178,176,173,
	170,168,165,162,160,157,154,152,149,147,144,141,139,136,134,131,
	128,126,123,121,118,116,113,111,109,106,104,102,99,97,95,92,
	90,88,86,84,82,79,77,75,73,71,69,67,66,64,62,60,
	58,57,55,53,52,50,48,47,45,44,42,41,39,38,37,35,
	34,33,32,30,29,28,27,26,25,24,23,22,21,20,19,18,
	17,17,16,15,14,14,13,12,12,11,10,10,9,9,8,8,
	7,7,6,6,6,5,5,4,4,4,4,3,3,3,3,2,
	2,2,2,2,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

extern const uint8_t pulse_2_4[256] PROGMEM =
{
	0,0,1,3,5,8,12,15,20,24,29,34,40,46,51,58,
	64,70,76,83,89,96,102,109,115,122,128,135,141,147,153,159,
	164,170,175,181,186,191,196,200,205,209,213,217,220,224,227,230,
	233,236,238,241,243,245,247,248,250,251,252,253,254,254,255,255,
	255,255,255,254,254,253,252,251,250,249,248,247,245,243,242,240,
	238,236,234,232,229,227,225,222,220,217,215,212,209,206,203,201,
	198,195,192,189,186,183,180,177,174,170,167,164,161,158,155,152,
	149,146,143,140,137,134,131,128,125,122,119,116,113,110,108,105,
	102,100,97,94,92,89,87,84,82,79,77,75,72,70,68,66,
	64,62,60,58,56,54,52,50,48,47,45,43,42,40,39,37,
	36,34,33,32,30,29,28,27,25,24,23,22,21,20,19,18,
	17,17,16,15,14,13,13,12,11,11,10,10,9,9,8,8,
	7,7,6,6,5,5,5,4,4,4,4,3,3,3,3,2,
	2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

extern const uint8_t pulse_2_6[256] PROGMEM =
{
	0,0,0,0,1,2,2,4,5,7,10,13,16,19,23,27,
	32,37,42,47,53,59,65,71,78,84,91,98,105,111,118,125,
	132,139,145,152,159,165,171,177,183,189,194,200,205,210,214,219,
	223,227,231,234,237,240,243,245,247,249,251,252,253,254,255,255,
	255,255,255,254,253,252,251,250,248,246,244,242,240,238,235,233,
	230,227,224,221,218,214,211,207,204,200,197,193,189,186,182,178,
	174,170,166,162,159,155,151,147,143,139,136,132,128,125,121,117,
	114,110,107,103,100,97,94,90,87,84,81,78,75,73,70,67,
	65,62,60,57,55,53,51,48,46,44,42,40,39,37,35,33,
	32,30,29,27,26,25,23,22,21,20,19,18,17,16,15,14,
	13,13,12,11,10,10,9,9,8,7,7,7,6,6,5,5,
	5,4,4,4,3,3,3,3,2,2,2,2,2,2,1,1,
	1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


extern const uint8_t gamma_2_2[256] PROGMEM =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,
	3,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,
	6,7,7,7,8,8,8,9,9,9,10,10,10,11,11,12,
	12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,
	20,20,21,21,22,23,23,24,24,25,26,26,27,27,28,29,
	29,30,31,32,32,33,34,34,35,36,37,37,38,39,40,41,
	41,42,43,44,45,46,46,47,48,49,50,51,52,53,54,55,
	55,56,57,58,59,60,61,62,63,64,65,67,68,69,70,71,
	72,73,74,75,76,78,79,80,81,82,83,85,86,87,88,89,
	91,92,93,94,96,97,98,100,101,102,104,105,106,108,109,110,
	112,113,115,116,117,119,120,122,123,125,126,128,129,131,132,134,
	135,137,139,140,142,143,145,147,148,150,151,153,155,156,158,160,
	161,163,165,167,168,170,172,174,175,177,179,181,183,185,186,188,
	190,192,194,196,198,200,201,203,205,207,209,211,213,215,217,219,
	221,223,225,227,229,232,234,236,238,240,242,244,246,248,251,253,
};
