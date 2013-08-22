#include "Bliplace2.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

//--------------------------------------------------------------------------------

#define bit(A)   (1 << A)
#define sbi(p,b) { p |= (unsigned char)bit(b); }
#define cbi(p,b) { p &= (unsigned char)~bit(b); }
#define tbi(p,b) { p ^= (unsigned char)bit(b); }
#define gbi(p,b) (p & (unsigned char)bit(b))

#define lo8(A) (((uint16_t)A) & 0xFF)
#define hi8(A) (((uint16_t)A) >> 8)

//--------------------------------------------------------------------------------
// audio processing configs

#define BLIP_TRIGGER1_MIN  50
#define BLIP_TRIGGER2_MIN  50

#define BRIGHT1_UP   (65535 / 30)
#define BRIGHT1_DOWN (65535 / 300)

#define BRIGHT2_UP   (65535 / 40)
#define BRIGHT2_DOWN (65535 / 700)

//--------------------------------------------------------------------------------
// Externally-visible data.

// Back buffer, standard RGB format.
Color blip_pixels[8];

// PWM cycle tick, 4.096 kilohertz.
uint32_t volatile blip_tick;

// Button debounce counters
volatile uint8_t buttonstate1 = 0;
volatile uint16_t debounce_up1 = 0;
volatile uint16_t debounce_down1 = 0;

volatile uint8_t buttonstate2 = 0;
volatile uint16_t debounce_up2 = 0;
volatile uint16_t debounce_down2 = 0;

uint8_t blip_history[512];

//--------------------------------------------------------------------------------
// Internal data

// Front buffer, bit-planes format.
uint8_t blip_bits_red[8];
uint8_t blip_bits_green[8];
uint8_t blip_bits_blue[8];

// Blanking interval flag.
volatile uint8_t blip_blank;

// Audio enable flag. If disabled, audio processing will keep using the previous
// sample.
uint8_t blip_audio_enable;

// Raw unfiltered ADC sample - this includes the microphone's DC bias.
uint16_t blip_sample;

// Treble channel trigger value.
uint16_t blip_trigger1 = BLIP_TRIGGER1_MIN;

// Bass channel trigger value.
uint16_t blip_trigger2 = BLIP_TRIGGER2_MIN;

// Current channel 1 sample, contains mostly treble frequencies.
uint16_t blip_sample1;

// Current channel 2 sample, contains mostly bass frequencies.
uint16_t blip_sample2;

// Microphone DC bias filter accumulator.
int16_t blip_filter_dc;

// Bass filter accumulator.
int16_t blip_filter_bass;

// Brightness cursor, treble channel.
uint16_t blip_bright1;

// Brightness cursor, channel 2.
uint16_t blip_bright2;

// Brightness accumulator, treble channel.
uint16_t blip_brightaccum1 = 0;

// Brightness accumulator, channel 2.
uint16_t blip_brightaccum2 = 0;

// PWM callback function pointer dispatched by the timer interrupt.
void (*blip_timer_callback)();

// Time when we last saw activity on the bass audio channel. If we haven't seen
// any activity in 5 minutes, we go to sleep.
uint32_t blip_idle = 0;

//-----------------------------------------------------------------------------
// Exponential ramp.

uint8_t extern const PROGMEM exptab[256]  =
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

// Unsigned sinusoid table.
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

// Signed sinusoid table.
extern const int8_t ssintab[256] PROGMEM =
{
   0,    3,    6,    9,   12,   15,   18,   21,   24,   27,   30,   33,   36,   39,   42,   45,
  48,   51,   54,   57,   59,   62,   65,   67,   70,   73,   75,   78,   80,   82,   85,   87,
  89,   91,   94,   96,   98,  100,  102,  103,  105,  107,  108,  110,  112,  113,  114,  116,
 117,  118,  119,  120,  121,  122,  123,  123,  124,  125,  125,  126,  126,  126,  126,  126,
 127,  126,  126,  126,  126,  126,  125,  125,  124,  123,  123,  122,  121,  120,  119,  118,
 117,  116,  114,  113,  112,  110,  108,  107,  105,  103,  102,  100,   98,   96,   94,   91,
  89,   87,   85,   82,   80,   78,   75,   73,   70,   67,   65,   62,   59,   57,   54,   51,
  48,   45,   42,   39,   36,   33,   30,   27,   24,   21,   18,   15,   12,    9,    6,    3,
   0,   -3,   -6,   -9,  -12,  -15,  -18,  -21,  -24,  -27,  -30,  -33,  -36,  -39,  -42,  -45,
 -48,  -51,  -54,  -57,  -59,  -62,  -65,  -67,  -70,  -73,  -75,  -78,  -80,  -82,  -85,  -87,
 -89,  -91,  -94,  -96,  -98, -100, -102, -103, -105, -107, -108, -110, -112, -113, -114, -116,
-117, -118, -119, -120, -121, -122, -123, -123, -124, -125, -125, -126, -126, -126, -126, -126,
-127, -126, -126, -126, -126, -126, -125, -125, -124, -123, -123, -122, -121, -120, -119, -118,
-117, -116, -114, -113, -112, -110, -108, -107, -105, -103, -102, -100,  -98,  -96,  -94,  -91,
 -89,  -87,  -85,  -82,  -80,  -78,  -75,  -73,  -70,  -67,  -65,  -62,  -59,  -57,  -54,  -51,
 -48,  -45,  -42,  -39,  -36,  -33,  -30,  -27,  -24,  -21,  -18,  -15,  -12,   -9,   -6,   -3,
};

// Square root.
extern const uint8_t root2[257] PROGMEM = {
  0,  15,  22,  27,  31,  35,  39,  42,  45,  47,  50,  52,  55,  57,  59,  61,
 63,  65,  67,  69,  71,  73,  74,  76,  78,  79,  81,  82,  84,  85,  87,  88,
 90,  91,  92,  94,  95,  96,  98,  99, 100, 102, 103, 104, 105, 106, 108, 109,
110, 111, 112, 113, 114, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 138, 139, 140, 141,
142, 143, 144, 145, 146, 146, 147, 148, 149, 150, 151, 152, 152, 153, 154, 155,
156, 156, 157, 158, 159, 160, 160, 161, 162, 163, 164, 164, 165, 166, 167, 167,
168, 169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 178, 179,
180, 181, 181, 182, 183, 183, 184, 185, 185, 186, 187, 187, 188, 189, 189, 190,
191, 191, 192, 193, 193, 194, 195, 195, 196, 197, 197, 198, 199, 199, 200, 200,
201, 202, 202, 203, 204, 204, 205, 205, 206, 207, 207, 208, 209, 209, 210, 210,
211, 212, 212, 213, 213, 214, 215, 215, 216, 216, 217, 217, 218, 219, 219, 220,
220, 221, 221, 222, 223, 223, 224, 224, 225, 225, 226, 227, 227, 228, 228, 229,
229, 230, 230, 231, 232, 232, 233, 233, 234, 234, 235, 235, 236, 236, 237, 237,
238, 239, 239, 240, 240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246,
246, 247, 247, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 254, 254,
255
};

// Cube root.
extern const uint8_t root3[257] PROGMEM = {
  0,  40,  50,  57,  63,  68,  72,  76,  80,  83,  86,  89,  91,  94,  96,  99,
101, 103, 105, 107, 109, 110, 112, 114, 115, 117, 118, 120, 121, 123, 124, 126,
127, 128, 130, 131, 132, 133, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 158, 159,
160, 161, 162, 163, 163, 164, 165, 166, 167, 167, 168, 169, 170, 170, 171, 172,
173, 173, 174, 175, 175, 176, 177, 177, 178, 179, 179, 180, 181, 181, 182, 183,
183, 184, 185, 185, 186, 187, 187, 188, 188, 189, 190, 190, 191, 191, 192, 193,
193, 194, 194, 195, 195, 196, 196, 197, 198, 198, 199, 199, 200, 200, 201, 201,
202, 202, 203, 203, 204, 204, 205, 206, 206, 207, 207, 208, 208, 209, 209, 210,
210, 210, 211, 211, 212, 212, 213, 213, 214, 214, 215, 215, 216, 216, 217, 217,
218, 218, 218, 219, 219, 220, 220, 221, 221, 222, 222, 222, 223, 223, 224, 224,
225, 225, 225, 226, 226, 227, 227, 228, 228, 228, 229, 229, 230, 230, 230, 231,
231, 232, 232, 232, 233, 233, 234, 234, 234, 235, 235, 236, 236, 236, 237, 237,
237, 238, 238, 239, 239, 239, 240, 240, 240, 241, 241, 242, 242, 242, 243, 243,
243, 244, 244, 244, 245, 245, 246, 246, 246, 247, 247, 247, 248, 248, 248, 249,
249, 249, 250, 250, 250, 251, 251, 251, 252, 252, 252, 253, 253, 253, 254, 254,
255,
};   

// White noise.
extern const uint8_t PROGMEM noise[256] =
{
  159,103,68,166,7,138,160,220,114,59,239,145,195,8,36,73,
  132,168,71,149,170,150,70,133,153,218,20,2,98,231,24,110,
  191,117,238,31,129,144,124,208,191,52,189,78,209,198,195,141,
  253,209,11,189,66,144,150,223,164,172,108,180,236,109,146,130,
  95,221,126,213,188,93,56,142,36,99,232,65,108,58,208,32,
  143,204,160,232,27,178,133,78,43,74,113,151,138,45,60,156,
  16,241,165,124,223,58,93,0,11,44,109,93,139,85,176,1,
  170,39,106,186,168,143,72,184,47,245,232,135,130,134,191,118,
  136,38,46,60,99,102,117,120,202,191,107,19,89,95,53,136,
  179,68,85,128,127,90,155,143,246,13,193,212,126,212,35,88,
  194,223,196,37,86,82,198,251,30,163,81,188,216,50,206,31,
  111,93,230,8,79,217,197,233,134,108,109,142,81,45,162,39,
  51,31,167,103,144,139,151,217,196,78,179,245,35,164,179,34,
  50,32,73,180,174,122,175,16,206,13,53,163,188,107,104,247,
  195,222,9,6,167,30,85,44,161,250,109,37,29,219,124,87,
  167,80,151,156,194,4,162,92,44,169,95,213,246,216,127,38
};

// Hue ramp, for hue-to-color conversion.
extern const uint8_t PROGMEM huetab[256] = {
  0,   5,  11,  17,  23,  29,  35,  41,  47,  53,  59,  65,  71,  77,  83,  89,
 95, 101, 107, 113, 119, 125, 131, 137, 143, 149, 155, 161, 167, 173, 179, 185,
191, 197, 203, 209, 215, 221, 227, 233, 239, 245, 251, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 249, 243, 237, 231, 225, 219, 213, 207, 201, 195, 189, 183, 177, 171, 165,
159, 153, 147, 141, 135, 129, 123, 117, 111, 105,  99,  93,  87,  81,  75,  69,
 63,  57,  51,  45,  39,  33,  27,  21,  15,   9,   3,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
}; 

//-----------------------------------------------------------------------------
// Xorshift RNG, from Wikipedia.

uint32_t xor128(void) {
  static uint32_t x = 123456789;
  static uint32_t y = 362436069;
  static uint32_t z = 521288629;
  static uint32_t w = 88675123;
  uint32_t t;
 
  t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
}

//-----------------------------------------------------------------------------
// Fractional multiply functions.

// Multiply two unsigned 16-bit fractions.
__attribute__((naked)) uint16_t mul_f16(uint16_t a, uint16_t b) {
  asm("clr r26");
  
  // r19:r18 = a
  asm("movw r18, r24");

  // r21:r20 = b  
  asm("movw r20, r22");
  
  // r25:r24:r23:r22 = scratch
  // scratch = ((signed)ah * (signed)bh) << 16;
  asm("mul r19, r21");
  asm("movw r24, r0");
  
  // scratch += al*bl
  asm("mul r18, r20");
  asm("movw r22, r0");
  
  // ah * bl
  asm("mul r19, r20");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  // bh * al
  asm("mul r21, r18");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  asm("clr r1");
  asm("ret");
  
  // Dummy return
  return 0;
}

// Multiply two signed 16-bit fractions.
__attribute__((naked)) int16_t mul_f16(int16_t a, int16_t b) {
  // r26 = 0
  asm("clr r26");

  // r19:r18 = a
  asm("movw r18, r24");

  // r21:r20 = b
  asm("movw r20, r22");
  
  // r25:r24:r23:r22 = scratch
  // scratch = ((signed)ah * (signed)bh) << 16;
  asm("muls r19, r21");
  asm("movw r24, r0");
  
  // scratch += al*bl
  asm("mul r18, r20");
  asm("movw r22, r0");
  
  // (signed)ah * bl
  asm("mulsu r19, r20");
  asm("sbc r25, r26");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  // (signed)bh * al
  asm("mulsu r21, r18");
  asm("sbc r25, r26");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  // The result effectively has two sign bits, so shift
  // it left 1 bit.  
  asm("rol r23");
  asm("rol r24");
  asm("rol r25");
  
  asm("clr r1");
  asm("ret");
  
  // Dummy return;
  return 0;
}

// Multiply one signed and one unsigned 16-bit fraction.
__attribute__((naked)) int16_t mul_f16(int16_t a, uint16_t b) {
  // r26 = 0
  asm("clr r26");

  // r19:r18 = a
  asm("movw r18, r24");

  // r21:r20 = b
  asm("movw r20, r22");
  
  // r25:r24:r23:r22 = scratch
  
  // scratch = ((signed)ah * bh) << 16;
  asm("mulsu r19, r21");
  asm("movw r24, r0");
  
  // scratch += al*bl
  asm("mul r18, r20");
  asm("movw r22, r0");
  
  // (signed)ah * bl
  asm("mulsu r19, r20");
  asm("sbc r25, r26");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  // bh * al
  asm("mul r21, r18");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");

  asm("clr r1");
  asm("ret");

  // Dummy return;
  return 0;
}  


// Multiply one unsigned and one signed 16-bit fraction.
__attribute__((naked)) int16_t mul_f16(uint16_t a, int16_t b) {
  // r26 = 0
  asm("clr r26");

  // r19:r18 = a
  asm("movw r18, r24");

  // r21:r20 = b
  asm("movw r20, r22");
  
  // r25:r24:r23:r22 = scratch
  // scratch = (ah * (signed)bh) << 16;
  asm("mulsu r21, r19");
  asm("movw r24, r0");
  
  // scratch += al*bl
  asm("mul r18, r20");
  asm("movw r22, r0");
  
  // ah * bl
  asm("mul r19, r20");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  // (signed)bh * al
  asm("mulsu r21, r18");
  asm("sbc r25, r26");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  asm("clr r1");
  asm("ret");

  // Dummy return;
  return 0;
}

//-----------------------------------------------------------------------------
// Interpolate between two values.

// Interpolate between two unsigned 8-bit numbers.
__attribute__((naked)) uint8_t lerp_u8(uint8_t x1, uint8_t x2, uint8_t t) {
  // r24:r23 is our scratch space.
  asm("mov r21, r24"); // x1 is now in r21
  
  // scratch = x2 * t
  asm("mul r22, r20");
  asm("mov r23, r0");
  asm("mov r24, r1");
  
  // t = ~t
  asm("com r20");
  
  // scratch += x1 * ~t
  asm("mul r21, r20");
  asm("add r23, r0");
  asm("adc r24, r1");

  asm("clr r1");  
  
  // scratch += x1
  asm("add r23, r21");
  asm("adc r24, r1");

  asm("ret");

  // Dummy return;
  return 0;
}  

// Interpolate between two signed 8-bit numbers.
__attribute__((naked)) int8_t lerp_s8(int8_t a, int8_t b, uint8_t t) {
  return lerp_u8(a ^ 0x80, b ^ 0x80, t) ^ 0x80;
}  

// Interpolate between two unsigned 16-bit numbers.
__attribute__((naked)) uint16_t lerp_u16(uint16_t x1, uint16_t x2, uint8_t t) {
  // r21 = 0
  // r20:r19:r18 is our scratch space.
  asm("mov r26, r20");
  asm("clr r20");
  asm("clr r21");
  
  // scratch = x2l * t
  asm("mul r22, r26");
  asm("movw r18, r0");
  
  // scratch += (x2h * t) << 8;
  asm("mul r23, r26");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // t = ~t
  asm("com r26");
  
  // scratch += x1l * ~t
  asm("mul r24, r26");
  asm("add r18, r0");
  asm("adc r19, r1");
  asm("adc r20, r21");
  
  // scratch += (x1h * ~t) << 8;
  asm("mul r25, r26");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // scratch += x1
  asm("add r18, r24");
  asm("adc r19, r25");
  asm("adc r20, r21");
  
  // result = scratch >> 8
  asm("mov r24, r19");
  asm("mov r25, r20");
  
  asm("clr r1");
  asm("ret");

  // Dummy return;
  return 0;
}

// Interpolate between two signed 16-bit numbers.
__attribute__((naked)) uint16_t lerp16_s16(int16_t a, int16_t b, uint8_t t) {
  return lerp_u16(a ^ 0x8000, b ^ 0x8000, t) ^ 0x8000;
}  

//-----------------------------------------------------------------------------
// Interpolated table lookup.

// Interpolate between elements in a 256-element, 8-bit table in flash, with
// wrapping.
__attribute__((naked)) uint8_t lerp_u8_u8(const uint8_t* table, uint16_t x) {
  // x1 = x & 0xff; (in r23)
  // t = x >> 8;  (in r22);
  
  // y1 = table[x1] (in r20);
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r20, z");

  // x1++;
  asm("inc r23");
  
  // y2 = table[x1]; (in r21);
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r21, z");
  
  // scratch = y2 * t
  asm("mul r21, r22");
  asm("mov r23, r0");  
  asm("mov r24, r1");
  
  // t = ~t
  asm("com r22");
  
  // scratch += y1 * ~t
  asm("mul r20, r22");
  asm("add r23, r0");
  asm("adc r24, r1");
  
  asm("clr r1");
  
  // scratch += y1
  asm("add r23, r20");
  asm("adc r24, r1");
  
  asm("ret");

  // Dummy return;
  return 0;
}


// Interpolate between elements in a 256-element, 8-bit table in flash, with
// wrapping, expanding the table value out to 16 bits.
__attribute__((naked)) uint16_t lerp_u8_u16(const uint8_t* table, uint16_t x) {
  asm("clr r20");
  asm("clr r21");
  
  // y1 = table[(x >> 8)]
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r26, z");

  // x1++;
  asm("inc r23");
  
  // y2 = table[(x >> 8) + 1];
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r27, z");
  
  // scratch = (y2 * 257) * t
  asm("mul r27, r22");
  asm("movw r18, r0");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // t = ~t
  asm("com r22");
  
  // scratch += (y1 * 257) * ~t
  asm("mul r26, r22");
  asm("add r18, r0");
  asm("adc r19, r1");
  asm("adc r20, r21");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // scratch += (y1 * 257)
  asm("add r18, r26");
  asm("adc r19, r26");
  asm("adc r20, r21");
  
  // result = scratch >> 8
  asm("mov r24, r19");
  asm("mov r25, r20");
  
  asm("clr r1");
  asm("ret");

  // Dummy return;
  return 0;
}


// Interpolate between elements in a 256-element, 8-bit table in RAM,
// with wrapping, expanding the table value out to 16 bits.
__attribute__((naked)) uint16_t lerp_u8_u16_ram(uint8_t* table, uint16_t x) {
  asm("clr r20");
  asm("clr r21");
  
  // y1 = table[(x >> 8)]
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("ld r26, z");

  // x1++;
  asm("inc r23");
  
  // y2 = table[(x >> 8) + 1];
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("ld r27, z");
  
  // scratch = (y2 * 257) * t
  asm("mul r27, r22");
  asm("movw r18, r0");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // t = ~t
  asm("com r22");
  
  // scratch += (y1 * 257) * ~t
  asm("mul r26, r22");
  asm("add r18, r0");
  asm("adc r19, r1");
  asm("adc r20, r21");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // scratch += (y1 * 257)
  asm("add r18, r26");
  asm("adc r19, r26");
  asm("adc r20, r21");
  
  // result = scratch >> 8
  asm("mov r24, r19");
  asm("mov r25, r20");
  
  asm("clr r1");
  asm("ret");

  // Dummy return;
  return 0;
}

// Interpolate between elements in a 256-element, signed 8-bit table, with wrapping,
// expanding the table values out to a signed 16-bit value.
// input r25:r24 - table
// input r23:r22 - x
// output r25:r24 - signed 16-bit interpolated table value.
__attribute__((naked)) int16_t lerp_s8_s16(const int8_t* table, uint16_t x) {
  // r26 = 0.
  asm("clr r26");
  
  // r19:r18 = y1 = expand(table[(x >> 8)])
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r18, z");
  asm("mov r19, r18");
  asm("sbrc r18, 7");
  asm("subi r19, 1");

  // x1++;
  asm("inc r23");
  
  // r21:r20 = y2 = expand(table[(x >> 8) + 1]);
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r20, z");
  asm("mov r21, r20");
  asm("sbrc r20, 7");
  asm("subi r21, 1");
  
  // r22 = t = interpolation factor
  // r25:r24:r23 = scratch
  
  // scratch = ((signed)y2h * t) << 8
  asm("mulsu r21, r22");
  asm("clr r23");
  asm("mov r24, r0");
  asm("mov r25, r1");
  
  // scratch += y2l * t
  asm("mul r20, r22");
  asm("mov r23, r0");
  asm("add r24, r1");
  asm("adc r25, r26");
  
  // t = ~t
  asm("com r22");
  
  // scratch += ((signed)y1h * t) << 8
  asm("mulsu r19, r22");
  asm("add r24, r0");
  asm("adc r25, r1");
  
  // scratch += y1l * t
  asm("mul r18, r22");
  asm("add r23, r0");
  asm("adc r24, r1");
  asm("adc r25, r26");
  
  // scratch += y1
  asm("add r23, r18");
  asm("adc r24, r19");
  asm("adc r25, r26");
  
  asm("clr r1");
  asm("ret");

  // Dummy return;
  return 0;
}


int16_t lerp_s8_s16(const int8_t* table, int16_t x) {
  return lerp_s8_s16(table, uint16_t(x ^ 0x8000));
}  


// Interpolate between elements in a 257-element, 8-bit table, no wrapping,
// expanding the table value out to 16 bits.
__attribute__((naked)) uint16_t lerp_u8_u16_nowrap(const uint8_t* table, uint16_t x) {
  asm("clr r20");
  asm("clr r21");
  
  // y1 = table[(x >> 8)]
  // y2 = table[(x >> 8) + 1];
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("lpm r26, z+");
  asm("lpm r27, z");

  // scratch = (y2 * 257) * t
  asm("mul r27, r22");
  asm("movw r18, r0");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // t = ~t
  asm("com r22");
  
  // scratch += (y1 * 257) * ~t
  asm("mul r26, r22");
  asm("add r18, r0");
  asm("adc r19, r1");
  asm("adc r20, r21");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // scratch += (y1 * 257)
  asm("add r18, r26");
  asm("adc r19, r26");
  asm("adc r20, r21");
  
  // result = scratch >> 8
  asm("mov r24, r19");
  asm("mov r25, r20");
  
  asm("clr r1");
  asm("ret");

  // Dummy return;
  return 0;
}


uint8_t lerp_u8_u8_nowrap(const uint8_t* table, uint16_t x) {
  return lerp_u8_u16_nowrap(table, x) >> 8;
}  


// Interpolate between elements in a 16-bit table, no wrapping.
__attribute__((naked)) uint16_t lerp_u16_u16_nowrap(const uint16_t* table, uint16_t x) {
  // scratch = 0;
  asm("clr r20");
  asm("clr r21");
  
  // table += (x >> 8) * 2
  asm("movw r30, r24");
  asm("add r30, r23");
  asm("adc r31, r1");
  asm("add r30, r23");
  asm("adc r31, r1");

  // y1 = table[0];
  // y2 = table[1];
  asm("lpm r24, z+");
  asm("lpm r25, z+");
  asm("lpm r26, z+");
  asm("lpm r27, z");

  // scratch = y2.l * t
  asm("mul r26, r22");
  asm("movw r18, r0");
  
  // scratch += (y2.h * t) << 8;
  asm("mul r27, r22");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // t = ~t
  asm("com r22");
  
  // scratch += y1.l * ~t
  asm("mul r24, r22");
  asm("add r18, r0");
  asm("adc r19, r1");
  asm("adc r20, r21");
  
  // scratch += (y1.h * ~t) << 8;
  asm("mul r25, r22");
  asm("add r19, r0");
  asm("adc r20, r1");
  
  // scratch += y1
  asm("add r18, r24");
  asm("adc r19, r25");
  asm("adc r20, r21");
  
  // result = scratch >> 8
  asm("mov r24, r19");
  asm("mov r25, r20");
  
  asm("clr r1");
  asm("ret");

  // Dummy return;
  return 0;
}  

//-----------------------------------------------------------------------------
// Color class

uint8_t hex2dec(char code) {
  uint8_t x = code - 48;
  if(x > 9) x -= 7;
  if(x > 15) x -= 32;
  return x;
}

Color blip_color(uint16_t r, uint16_t g, uint16_t b) {
  Color result = { r, g, b };
  return result;
}  

Color blip_color(float r, float g, float b) {
  Color result = {
    uint16_t(r * 65535),
    uint16_t(g * 65535),
    uint16_t(b * 65535)
  };
  return result;
}  

Color blip_color(uint16_t hue) {
  Color result = {
    blip_hsv_r(hue),
    blip_hsv_g(hue),
    blip_hsv_b(hue)
  };
  return result;
}

Color blip_color(float hue) {
  return blip_color(hue * 65535);
}  

Color blip_color(const char* hexcode) {
  uint8_t r, g, b;
  
  if (hexcode[3] == 0) {
    // rgb
    r = hex2dec(hexcode[0]);
    g = hex2dec(hexcode[1]);
    b = hex2dec(hexcode[2]);
    r |= r << 4;
    g |= g << 4;
    b |= b << 4;
  }
  else if (hexcode[4] == 0) {
    // #rgb
    r = hex2dec(hexcode[1]);
    g = hex2dec(hexcode[2]);
    b = hex2dec(hexcode[3]);
    r |= r << 4;
    g |= g << 4;
    b |= b << 4;
  }    
  else if(hexcode[0] == '#') {
    // #rrggbb
    r = hex2dec(hexcode[1]) * 16 + hex2dec(hexcode[2]);
    g = hex2dec(hexcode[3]) * 16 + hex2dec(hexcode[4]);
    b = hex2dec(hexcode[5]) * 16 + hex2dec(hexcode[6]);
  }
  else {
    // rrggbb
    r = hex2dec(hexcode[0]) * 16 + hex2dec(hexcode[1]);
    g = hex2dec(hexcode[2]) * 16 + hex2dec(hexcode[3]);
    b = hex2dec(hexcode[4]) * 16 + hex2dec(hexcode[5]);
  }
  
  Color result = {
    uint16_t(r | r << 8),
    uint16_t(g | g << 8),
    uint16_t(b | b << 8)
  };
  return result;
}

//-----------------------------------------------------------------------------
// Bliplace API, scalar functions.

uint16_t blip_time() {
  return blip_tick / 4096;
}

uint16_t blip_tempo(uint16_t bpm) {
  return (blip_tick * bpm * 17476) >> 16;
}  

uint16_t blip_sin(uint16_t x) {
  return lerp_u8_u16(sintab, x);
}

uint16_t blip_cos(uint16_t x) {
  return lerp_u8_u16(sintab, x + 16384);
}

uint16_t blip_halfsin(uint16_t x) {
  return (blip_sin(x / 2) - 32768) * 2;
}  

int16_t blip_ssin(uint16_t x) {
  return blip_sin(x) ^ 0x8000;
}  

int16_t blip_scos(uint16_t x) {
  return blip_cos(x) ^ 0x8000;
}  

uint16_t blip_scale(uint16_t x, uint16_t s) {
  return mul_f16(x, s);
}

int16_t blip_scale(int16_t x, uint16_t s) {
  return mul_f16(x, s);
}

// 'Smooth' add - doesn't overflow.
uint16_t blip_smadd(uint16_t a, uint16_t b) {
  return a + b - mul_f16(a, b);
}  

uint16_t blip_hsv_r(uint16_t h) {
  return lerp_u8_u16(huetab, h);
}

uint16_t blip_hsv_g(uint16_t h) {
  return lerp_u8_u16(huetab, h + 21845);
}

uint16_t blip_hsv_b(uint16_t h) {
  return lerp_u8_u16(huetab, h + 43690);
}

uint16_t blip_pow2(uint16_t x) {
  return blip_scale(x, x);
}

uint16_t blip_pow3(uint16_t x) {
  uint16_t x2 = blip_scale(x, x);
  return blip_scale(x, x2);
}
  
uint16_t blip_pow4(uint16_t x) {
  uint16_t x2 = blip_scale(x, x);
  return blip_scale(x2, x2);
}
  
uint16_t blip_pow5(uint16_t x) {
  uint16_t x2 = blip_scale(x, x);
  uint16_t x3 = blip_scale(x, x2);
  return blip_scale(x2, x3);
}  

uint16_t blip_pow6(uint16_t x) {
  uint16_t x2 = blip_scale(x, x);
  uint16_t x3 = blip_scale(x, x2);
  return blip_scale(x3, x3);
}  

uint16_t blip_pow7(uint16_t x) {
  uint16_t x2 = blip_scale(x, x);
  uint16_t x3 = blip_scale(x, x2);
  uint16_t x4 = blip_scale(x2, x2);
  return blip_scale(x3, x4);
}

uint16_t blip_root2(uint16_t x) {
  return lerp_u8_u16_nowrap(root2, x);
}

uint16_t blip_root3(uint16_t x) {
  return lerp_u8_u16_nowrap(root3, x);
}

uint16_t blip_root4(uint16_t x) {
  return blip_root2(blip_root2(x));
}

uint16_t blip_root6(uint16_t x) {
  return blip_root3(blip_root2(x));
}

uint16_t blip_root9(uint16_t x) {
  return blip_root3(blip_root3(x));
}  

uint16_t blip_noise(uint16_t x) {
  return lerp_u8_u16(noise, x);
}

uint16_t blip_lookup(const uint8_t* table, uint16_t x) {
  return lerp_u8_u16(table, x);
}

uint16_t blip_random() {
  return (uint16_t)xor128();
}

uint16_t blip_history1(uint16_t x) {
  uint16_t cursor = blip_tick;
  cursor -= x;
  return lerp_u8_u16_ram(blip_history, cursor);
}

uint16_t blip_history2(uint16_t x) {
  uint16_t cursor = blip_tick;
  cursor -= x;
  return lerp_u8_u16_ram(blip_history + 256, cursor);
}

//-----------------------------------------------------------------------------
// Bliplace API, color functions.

Color blip_scale(Color const& c, uint16_t s) {
  Color result = {
    blip_scale(c.r, s),
    blip_scale(c.g, s),
    blip_scale(c.b, s)
  };
  return result;
}


Color blip_scale(Color const& c, uint16_t s1, uint16_t s2) {
  Color result = {
    blip_scale(blip_scale(c.r, s1), s2),
    blip_scale(blip_scale(c.g, s1), s2),
    blip_scale(blip_scale(c.b, s1), s2)
  };
  return result;
}

Color blip_add(Color const& a, Color const& b) {
  Color result = {
    a.r + b.r,
    a.g + b.g,
    a.b + b.b
  };
  return result;
}

Color blip_smadd(Color const& a, Color const& b) {
  Color result = {
    blip_smadd(a.r, b.r),
    blip_smadd(a.g, b.g),
    blip_smadd(a.b, b.b)
  };
  return result;
}

Color blip_lerp(Color const& a, Color const& b, uint16_t x) {
  uint8_t t = x >> 8;
  Color result = {
    lerp_u16(a.r, b.r, t),
    lerp_u16(a.g, b.g, t),
    lerp_u16(a.b, b.b, t)
  };
  return result;
}

//-----------------------------------------------------------------------------
// Drawing function

void blip_draw_sin(uint16_t phase, uint16_t frequency, Color color) {
  uint16_t x = phase;
  for (int i = 0; i < 8; i++) {
    blip_pixels[i] = blip_scale(color, blip_sin(x));
    x += frequency;
  }
}

//---------------------------------------------------------------------------------
// Timer interrupt, dispatches our LED update callback. Note that we _ijmp_ to
// the callback, and _reti_ from the callback - this saves a few cycles per
// interrupt, which ends up being a big win when we're doing 24000 of them per
// second.

ISR(TIMER1_OVF_vect, ISR_NAKED)
{
  // We have to clobber registers R30 and R31 in order to dispatch the interrupt
  // callback, so we're required to save them.
  asm("push r30");
  asm("push r31");
  
  // Loading the timer here produces values in r25 of 12, 13, 14, 15 - which
  // map to jitter compensation values of 0, 1, 2, and 3. LDS doesn't touch
  // the status register, so this is safe to do before we've saved a copy of
  // it.
  asm("lds r31, %0" : : "M" (&TCNT1L));
  
  // Save the status register on the stack.
  asm("in r30, %0" : : "I"(_SFR_IO_ADDR(SREG)) );
  asm("push r30");
  
  // Load the low byte of the timer callback and apply jitter compensation to
  // it - we don't always jump directly to the interrupt, sometimes we jump
  // past a few NOPs. Since all interrupts are aligned(4), we don't have to do
  // a carry after the add.
  asm("lds r30, blip_timer_callback");
  asm("andi r31, 3");
  asm("add r30, r31");

  // Load the high byte of the interrupt callback now that we're done using
  // r31.
  asm("lds r31, blip_timer_callback+1");

  // Call interrupt callback
  asm("ijmp");
}

//---------------------------------------------------------------------------------
// Interrupt handlers

// Each callback needs to fire a precise number of cycles after the previous
// callback finished. These constants represent the amount of delay needed
// between adjacent callbacks so that everything stays in sync. Since our timer
// is counting up to 65536, we represent a delay of N as 65536 - N.

#define RED_FIELD_A_TIMEOUT   (65536 - 121)
#define RED_FIELD_B_TIMEOUT   (65536 - 294)

#define GREEN_FIELD_A_TIMEOUT (65536 - 121)
#define GREEN_FIELD_B_TIMEOUT (65536 - 294)

#define BLUE_FIELD_A_TIMEOUT  (65536 - 121)
#define BLUE_FIELD_B_TIMEOUT  (65536 - 294)

// Our PWM period is divided into 3 fields for red/green/blue, each of which is
// in turn divided into 2 callbacks for the 'low' bits 0-6 and the 'high' bit 7.
// We interleave our audio processing code between the LED pulses for bits 0-5,
// and we release the CPU back to the user after sending pulses for bits 6 and 7.

// Each field is 3+5+10+20+40+80+160+320 = 638 cycles long, and we have 3 of them
// per period - that's 1914 cycles.
// Our CPU runs at 8 mhz, and we want a refresh rate of 4096 hz - that's 1953
// cycles. We lose a few cycles in overhead between fields, but we still end up
// with ~20 cycles of dead space each period. We could probably do something more
// useful there.

// Note: absolutely everything in these callbacks _must_ run in _exactly_ the same
// number of cycles - all possible paths through branching code must be timed and
// padded with NOPs to keep the branch lengths equal.

// Sends pulses for red field bits 0 - 6 to the LEDs, interleaving those pulses
// with our audio processing code. Since this is the first callback fired for
// a PWM period, we also do a small amount of additional bookkeeping.

extern "C" {
__attribute__((naked, aligned(4))) void red_field_A() {
  // jitter padding
  asm("nop");  
  asm("nop");  
  asm("nop");  
  
  // end previous pulse
  {
    asm("clr r30");
    asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SOURCE)) );
  }		

  // 20 cycle dead space between PWM cycles. Time spent here should cause
  // the overall PWM rate to be 4.096 khz, or ~1953 cycles.
  
  // clear blanking flag (2 cycles)
  asm("sts blip_blank, r30");

  // 8000000 / 4096 = 1953.125 - to account for the 0.125, every eigth PWM
  // cycle is 1954 cycles instead of 1953. Yes, doing this correction is
  // ridiculous, especially considering that the error on the internal
  // oscillator masks any real correction we do. So what. Knowing that we run
  // at a theoretically precise 4.096 khz makes me happy, and if we ever do
  // switch to using a crystal then this will be precise for real.
    
  asm("lds r30, blip_tick + 0");
  asm("andi r30, 7");
  asm("breq eighth_cycle");
  asm("eighth_cycle:");
    
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");

  // switch to new sink
  asm("ldi r30, %0" : : "M" (SINK_RED));
  asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SINK)) );
  
  // send 3 cycle pulse
  {
    asm("lds r30, blip_bits_red + 0");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }		
  
  // send 6 cycle pulse
  {
    asm("lds r30, blip_bits_red + 1");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 3 cycle gap

  asm("nop"); asm("nop"); asm("nop");
  
  // send 11 cycle pulse
  {
    asm("lds r30, blip_bits_red + 2");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 8 cycle gap
  
  // save temp registers
  asm("push r25");
  asm("push r28");
  asm("push r29");
  asm("nop");
  asm("nop");
  
  // send 20 cycle pulse
  {
    asm("lds r25, blip_bits_red + 3");
    asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 17 cycle gap
  
  // save extra temp registers
  asm("push r26");
  asm("push r27");
  
  // remove dc bias, part 1
  {
    // blip_sample1 -= blip_filter_dc;
    // blip_filter_dc += (blip_sample1 >> DCFILTER);

    asm("lds r28, blip_sample + 0");
    asm("lds r29, blip_sample + 1");
    asm("lds r30, blip_filter_dc + 0");
    asm("lds r31, blip_filter_dc + 1");
  
    asm("sub r28, r30");
    asm("sbc r29, r31");
  
    asm("movw r26, r28");
    asm("asr r27"); asm("ror r26");
  }		

  // send 40 cycle pulse
  {
    asm("lds r25, blip_bits_red + 4");
    asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  
  // remove dc bias, part 2.
  {		
    asm("asr r27"); asm("ror r26");
    asm("asr r27"); asm("ror r26");
    asm("asr r27"); asm("ror r26");
    asm("asr r27"); asm("ror r26");
  
    asm("add r30, r26");
    asm("adc r31, r27");
  
    asm("sts blip_filter_dc + 0, r30");
    asm("sts blip_filter_dc + 1, r31");
  }		
  
  // split bass and treble
  {
    // blip_sample1 -= blip_filter_bass;
    // blip_filter_bass += (blip_sample1 >> BASSFILTER);
  
    asm("lds r30, blip_filter_bass + 0");
    asm("lds r31, blip_filter_bass + 1");
    asm("sub r28, r30");
    asm("sbc r29, r31");
  
    asm("movw r26, r28");
    asm("asr r27"); asm("ror r26");
    asm("asr r27"); asm("ror r26");
    asm("asr r27"); asm("ror r26");
  
    asm("add r30, r26");
    asm("adc r31, r27");
  
    asm("sts blip_filter_bass + 0, r30");
    asm("sts blip_filter_bass + 1, r31");
  }		

  asm("nop"); asm("nop"); asm("nop"); asm("nop"); 
  
  // send 80 cycle pulse
  {
    asm("lds r25, blip_bits_red + 5");
    asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }

  // normalize blip_sample2
  {
    // blip_sample2 = (blip_filter_bass < 0) ? -blip_filter_bass : blip_filter_bass;
    asm("sbrc r31, 7");
    asm("jmp negate_bass");
    asm("nop");
    asm("jmp bass_done");
    asm("negate_bass:");
    asm("com r31");
    asm("neg r30");
    asm("sbci r31, 0xFF");
    asm("bass_done:");
  
    asm("sts blip_sample2 + 0, r30");
    asm("sts blip_sample2 + 1, r31");
  }		

  // normalize treble
  {	
    // if(blip_sample1 < 0) blip_sample1 = -blip_sample1;
    asm("sbrc 29, 7");
    asm("jmp negate_treb");
    asm("nop");
    asm("jmp treb_done");
    asm("negate_treb:");
    asm("com r29");
    asm("neg r28");
    asm("sbci r29, 0xFF");
    asm("treb_done:");
    
    // store blip_sample1.
    asm("sts blip_sample1 + 0, r28");
    asm("sts blip_sample1 + 1, r29");
  }		
  
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  
  // adapt1
  {
    // if((blip_tick) & 63 == 0) adapt();
    asm("lds r30, blip_tick");
    asm("andi r30, 63");
    asm("breq trig1_adapt");
    
    // Cycle padding if we're not doing adaptation this period.
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop");
    asm("rjmp trig1_noadapt");
  
    asm("trig1_adapt:");
    asm("lds r30, blip_trigger1 + 0");
    asm("lds r31, blip_trigger1 + 1");
    asm("mov r28, r31");
    asm("lsr r28");
    asm("lsr r28");
  
    // Increase trigger if accum >= 4096, otherwise decrease.
    asm("lds r29, blip_brightaccum1 + 1");
    asm("andi r29, 0xF0");
    asm("breq trig1_down");
  
    asm("trig1_up:");
    asm("clr r29");
    asm("adiw r30, 1");
    asm("add r30, r28");
    asm("adc r31, r29");
    asm("jmp trig1_done");
  
    asm("trig1_down:");
    asm("clr r29");
    asm("sub r30, r28");
    asm("sbc r31, r29");
    asm("sbiw r30, 1");
    asm("nop");
    asm("nop");
  
    asm("trig1_done:");
    asm("sts blip_trigger1 + 0, r30");
    asm("sts blip_trigger1 + 1, r31");
  
    asm("trig1_noadapt:");
  }		

  // restore extra temp registers
  asm("pop r27");
  asm("pop r26");

  // restore temp registers
  asm("pop r29");
  asm("pop r28");
  asm("pop r25");
  
  // set next callback, 6 cycles
  asm("ldi r30, pm_lo8(red_field_B)");
  asm("ldi r31, pm_hi8(red_field_B)");
  asm("sts blip_timer_callback, r30");
  asm("sts blip_timer_callback+1, r31");
  
  // set next timeout, 6 cycles
  asm("ldi r30, %0" : : "M" (lo8(RED_FIELD_A_TIMEOUT)) );
  asm("ldi r31, %0" : : "M" (hi8(RED_FIELD_A_TIMEOUT)) );
  asm("sts %0, r31" : : "M" (&TCNT1H));
  asm("sts %0, r30" : : "M" (&TCNT1L));

  // send 160 cycle pulse
  {
    asm("lds r30, blip_bits_red + 6");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }		

  // Restore status register, R31, and R30.
  asm("pop r30");
  asm("out 0x3f, r30");
  asm("pop r31");
  asm("pop r30");

  // Done
  asm("reti");
}
};

//---------------------------------------------------------------------------------
// Sends red field bit 7 pulse and queues callback for green field.

extern "C" {
__attribute__((naked, aligned(4))) void red_field_B() {
  // jitter padding
  asm("nop");  
  asm("nop");  
  asm("nop");  

  // set next callback
  asm("ldi r30, pm_lo8(green_field_A)");
  asm("ldi r31, pm_hi8(green_field_A)");
  asm("sts blip_timer_callback, r30");
  asm("sts blip_timer_callback+1, r31");

  // set next timeout
  asm("ldi r30, %0" : : "M" (lo8(RED_FIELD_B_TIMEOUT)) );
  asm("ldi r31, %0" : : "M" (hi8(RED_FIELD_B_TIMEOUT)) );
  asm("sts %0, r31" : : "M" (&TCNT1H));
  asm("sts %0, r30" : : "M" (&TCNT1L));

  // send 320 cycle pulse
  {
    asm("lds r30, blip_bits_red + 7");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }		
  
  // Restore status register, R31, and R30.
  asm("pop r30");
  asm("out 0x3f, r30");
  asm("pop r31");
  asm("pop r30");

  // Done
  asm("reti");
}
};

//---------------------------------------------------------------------------------
// Green field A. Pulses and audio.

extern "C" {
__attribute__((naked, aligned(4))) void green_field_A() {
  // jitter padding
  asm("nop");  
  asm("nop");  
  asm("nop");  

  // end previous pulse
  {
    asm("clr r30");
    asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SOURCE)) );
  }		

  // switch to new sink
  asm("ldi r30, %0" : : "M" (SINK_GREEN));
  asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SINK)) );
  
  // send 3 cycle pulse
  {
    asm("lds r30, blip_bits_green + 0");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }		
  
  // send 6 cycle pulse
  {
    asm("lds r30, blip_bits_green + 1");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 3 cycle gap
  
  // clear the monitor bit (2 cycles)
  //asm("cbi %0, 2" : : "I"(_SFR_IO_ADDR(PORT_STATUS)) );
  asm("nop");
  asm("nop");
  asm("nop");
  
  // send 11 cycle pulse
  {
    asm("lds r30, blip_bits_green + 2");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }

  // 8 cycle gap
  // save temp registers
  asm("push r25");
  asm("push r28");
  asm("push r29");
  asm("nop");
  asm("nop");
  
  // send 20 cycle pulse
  {
    asm("lds r25, blip_bits_green + 3");
    asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 17 cycle gap.
  
#ifdef BLIP_NO_HISTORY
  asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
  asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
  asm("nop"); asm("snop"); asm("nop"); asm("nop"); asm("nop");
#else 
  // Store the old brightness values in the history buffer. The channel 1 and
  // channel 2 buffers are contiguous in memory, which saves us a few cycles.
  // int cursor = ((blip_tick >> 8) + 1) & 0xFF;
  // blip_history[cursor] = (blip_bright1 >> 8);
  // blip_history[cursor + 256] = (blip_bright2 >> 8);
    
  asm("lds r30, blip_tick + 1");
  asm("inc r30");
  asm("ldi r31, 0x00");
  asm("subi r30, lo8(-(blip_history))");
  asm("sbci r31, hi8(-(blip_history))");
    
  asm("lds r25, blip_bright1 + 1");
  asm("st z, r25");
  asm("inc r31");
  asm("lds r25, blip_bright2 + 1");
  asm("st z, r25");
#endif

  asm("nop");
  asm("nop");
  
  // send 40 cycle pulse
  {
    asm("lds r25, blip_bits_green + 4");
    asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 37 cycle gap

  // clamp 2, 18 cycles
  {
    // if(blip_trigger2 & 0x8000) blip_trigger2 -= 256;
    // if(blip_trigger2 < BLIP_TRIGGER2_MIN) blip_trigger2++;
    // 18 cycles
  
    asm("lds r30, blip_trigger2 + 0");
    asm("lds r31, blip_trigger2 + 1");

    // Our normalized audio sample is in the range (0,32786) - the trigger
    // should never go over 32768, so we clamp it by subtracting off 256 if
    // it does.
    asm("sbrc r31, 7");
    asm("subi r31, 0x01");
  
    // Clamp if below 60
    asm("clr r29");
    asm("cpi r30, %0" : : "I"(BLIP_TRIGGER2_MIN));
    asm("cpc r31, r29");
    {
      asm("brge trig2_noclamp");
      asm("adiw r30, 1");
      asm("rjmp trig2_clampdone");
  
      asm("trig2_noclamp:");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("trig2_clampdone:");
    }			
  
    asm("sts blip_trigger2 + 0, r30");
    asm("sts blip_trigger2 + 1, r31");
  }		

  // clamp 1, 18 cycles
  {
    // if(blip_trigger1 & 0x8000) blip_trigger1 -= 256;
    // if(blip_trigger1 < BLIP_TRIGGER1_MIN) blip_trigger1++;
    // 18 cycles

    asm("lds r30, blip_trigger1 + 0");
    asm("lds r31, blip_trigger1 + 1");

    // Clamp if above 32767
    asm("sbrc r31, 7");
    asm("subi r31, 0x01");
  
    // Clamp if below 60
    asm("clr r29");
    asm("cpi r30, %0" : : "I"(BLIP_TRIGGER1_MIN));
    asm("cpc r31, r29");
    {
      asm("brge trig1_noclamp");
      asm("adiw r30, 1");
      asm("rjmp trig1_clampdone");
  
      asm("trig1_noclamp:");
      asm("nop");
      asm("nop");
      asm("nop");
      asm("trig1_clampdone:");
    }			
  
    asm("sts blip_trigger1 + 0, r30");
    asm("sts blip_trigger1 + 1, r31");
  }		

  // 1 spare cycle
  asm("nop");

  // send 80 cycle pulse
  {
    asm("lds r25, blip_bits_green + 5");
    asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  
  // adapt 2
  {
    // if((blip_tick) & 63 == 0) adapt();
    asm("lds r30, blip_tick");
    asm("andi r30, 63");
    asm("breq trig2_adapt");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop");
    asm("rjmp trig2_noadapt");
  
    asm("trig2_adapt:");
    asm("lds r30, blip_trigger2 + 0");
    asm("lds r31, blip_trigger2 + 1");
    asm("mov r28, r31");
    asm("lsr r28");
    asm("lsr r28");
  
    // Increase trigger if accum >= 4096, otherwise decrease.
    asm("lds r29, blip_brightaccum2 + 1");
    asm("andi r29, 0xF0");
    asm("breq trig2_down");
  
    asm("trig2_up:");
    asm("clr r29");
    asm("adiw r30, 1");
    asm("add r30, r28");
    asm("adc r31, r29");
    asm("jmp trig2_done");
  
    asm("trig2_down:");
    asm("clr r29");
    asm("sub r30, r28");
    asm("sbc r31, r29");
    asm("sbiw r30, 1");
    asm("nop");
    asm("nop");
  
    asm("trig2_done:");
    asm("sts blip_trigger2 + 0, r30");
    asm("sts blip_trigger2 + 1, r31");
  
    asm("trig2_noadapt:");
  }		

  // clear brightness accumulators when the tick rolls over.
  // 14 cycles
  {
    // if((blip_tick & 63) == 0) { blip_brightaccum1 = 0; blip_brightaccum2 = 0; }
    asm("lds r30, blip_tick");
    asm("andi r30, 63");
    asm("brne noclear");
    asm("sts blip_brightaccum1 + 0, r30");
    asm("sts blip_brightaccum1 + 1, r30");
    asm("sts blip_brightaccum2 + 0, r30");
    asm("sts blip_brightaccum2 + 1, r30");
    asm("rjmp cleardone");
    asm("noclear:");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("cleardone:");
  }		

  // increment LED tick, 21 cycles
  {
    asm("clr r25");
    asm("lds r28, blip_tick+0");
    asm("lds r29, blip_tick+1");
    asm("lds r30, blip_tick+2");
    asm("lds r31, blip_tick+3");
    asm("adiw r28, 0x01");
    asm("adc r30, r25");
    asm("adc r31, r25");
    asm("sts blip_tick+0, r28");
    asm("sts blip_tick+1, r29");
    asm("sts blip_tick+2, r30");
    asm("sts blip_tick+3, r31");
  }
  asm("nop"); asm("nop");
  
  // set next callback
  asm("ldi r30, pm_lo8(green_field_B)");
  asm("ldi r31, pm_hi8(green_field_B)");
  asm("sts blip_timer_callback, r30");
  asm("sts blip_timer_callback+1, r31");
  
  // set next timeout
  asm("ldi r30, %0" : : "M" (lo8(GREEN_FIELD_A_TIMEOUT)) );
  asm("ldi r31, %0" : : "M" (hi8(GREEN_FIELD_A_TIMEOUT)) );
  asm("sts %0, r31" : : "M" (&TCNT1H));
  asm("sts %0, r30" : : "M" (&TCNT1L));
  
  // send 160 cycle pulse
  {
    asm("lds r30, blip_bits_green + 6");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }		

  // restore temp registers
  asm("pop r29");
  asm("pop r28");
  asm("pop r25");
  
  // Restore status register, R31, and R30.
  asm("pop r30");
  asm("out 0x3f, r30");
  asm("pop r31");
  asm("pop r30");

  // Done
  asm("reti");
}
}; 

//---------------------------------------------------------------------------------
// Green field B. LED pulse for green bit 7, that's it.

extern "C" {
__attribute__((naked, aligned(4))) void green_field_B() {
  // jitter padding
  asm("nop");  
  asm("nop");  
  asm("nop");  
  
  // set next callback
  asm("ldi r30, pm_lo8(blue_field_A)");
  asm("ldi r31, pm_hi8(blue_field_A)");
  asm("sts blip_timer_callback, r30");
  asm("sts blip_timer_callback+1, r31");

  // set next timeout
  asm("ldi r30, %0" : : "M" (lo8(GREEN_FIELD_B_TIMEOUT)) );
  asm("ldi r31, %0" : : "M" (hi8(GREEN_FIELD_B_TIMEOUT)) );
  asm("sts %0, r31" : : "M" (&TCNT1H));
  asm("sts %0, r30" : : "M" (&TCNT1L));
  
  // send 320 cycle pulse
  {
    asm("lds r30, blip_bits_green + 7");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }		

  // Restore status register, R31, and R30.
  asm("pop r30");
  asm("out 0x3f, r30");
  asm("pop r31");
  asm("pop r30");

  // Done
  asm("reti");
}
};

//---------------------------------------------------------------------------------
// Blue field A. More LED pulses as above, the last of the audio processing, and
// button updating go here.

extern "C" {
__attribute__((naked, aligned(4))) void blue_field_A() {
  // jitter padding
  asm("nop");  
  asm("nop");  
  asm("nop");  

  // end previous pulse
  {
    asm("clr r30");
    asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SOURCE)) );
  }		
  
  // switch to new sink
  asm("ldi r30, %0" : : "M" (SINK_BLUE));
  asm("out %0, r30" : : "I" (_SFR_IO_ADDR(PORT_SINK)) );
  
  // send 3 cycle pulse
  {
    asm("lds r30, blip_bits_blue + 0");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }		
  
  // send 6 cycle pulse
  {
    asm("lds r30, blip_bits_blue + 1");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 3 cycle gap

  asm("nop"); asm("nop"); asm("nop");
  
  // send 11 cycle pulse
  {
    asm("lds r30, blip_bits_blue + 2");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 8 cycle gap

  // save temp registers
  asm("push r25");
  asm("push r28");
  asm("push r29");
    
  asm("nop");
  asm("nop");
  
  // send 20 cycle pulse
  {
    asm("lds r25, blip_bits_blue + 3");
    asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 17 cycle gap
  
  // 14 cycles
  {
    // uint16_t temp = blip_bright1;
    // if(blip_sample1 >= blip_trigger1)

    asm("lds r28, blip_sample1 + 0");
    asm("lds r29, blip_sample1 + 1");
    asm("lds r30, blip_trigger1 + 0");
    asm("lds r31, blip_trigger1 + 1");
    asm("cp r28, r30");
    asm("cpc r29, r31");
    asm("lds r28, blip_bright1 + 0");
    asm("lds r29, blip_bright1 + 1");
  }

  asm("nop"); asm("nop"); asm("nop");
  
  // send 40 cycle pulse
  {
    asm("lds r25, blip_bits_blue + 4");
    asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 37 cycle gap

  // 12 cycles. don't split this block.
  {
    // if(blip_sample1 >= blip_trigger1)
    //     temp = (temp <= (65535 - BRIGHT1_UP)) ? temp + BRIGHT1_UP : 65535;
    // else
    //     temp = (temp >= BRIGHT1_DOWN) ? temp - BRIGHT1_DOWN : 0;
    asm("brcs signal_low1");
    asm("nop");
  
    asm("signal_high1:");
    asm("ldi r30, %0" : : "M"(lo8(65535 - BRIGHT1_UP)) );
    asm("ldi r31, %0" : : "M"(hi8(65535 - BRIGHT1_UP)) );
    asm("cp r28, r30");
    asm("cpc r29, r31");
    asm("brcc bright_max1");
    asm("subi r28, %0" : : "M"(lo8(-BRIGHT1_UP)) );
    asm("sbci r29, %0" : : "M"(hi8(-BRIGHT1_UP)) );
    asm("nop");
    asm("rjmp store_bright1");

    asm("signal_low1:");
    asm("ldi r30, %0" : : "M"(lo8(BRIGHT1_DOWN)) );
    asm("ldi r31, %0" : : "M"(hi8(BRIGHT1_DOWN)) );
    asm("cp r28, r30");
    asm("cpc r29, r31");
    asm("brcs bright_min1");
    asm("subi r28, %0" : : "M"(lo8(BRIGHT1_DOWN)) );
    asm("sbci r29, %0" : : "M"(hi8(BRIGHT1_DOWN)) );
    asm("nop");
    asm("rjmp store_bright1");

    asm("bright_max1:");
    asm("ser r28");
    asm("ser r29");
    asm("rjmp store_bright1");
  
    asm("bright_min1:");
    asm("clr r28");
    asm("clr r29");
    asm("rjmp store_bright1");
  }
  
  // 4 cycles
  {
    // blip_bright1 = temp;
    asm("store_bright1:");
    asm("sts blip_bright1 + 0, r28");
    asm("sts blip_bright1 + 1, r29");
  }

  // 9 cycles
  {
    // bright1 = pgm_read_byte(exptab+(blip_bright1 >> 8));
    asm("mov r30, r29");
    asm("ldi r31, 0x00");
    asm("subi r30, lo8(-(exptab))");
    asm("sbci r31, hi8(-(exptab))");
    asm("lpm r30, Z");
    asm("nop");
    asm("nop");
  }

  // 17 cycles (part 1 of 2, this part is 12 cycles)
  {
    // blip_brightaccum1 += pgm_read_byte(root2+bright1);
    asm("ldi r31, 0x00");
    asm("subi r30, lo8(-(root2))");
    asm("sbci r31, hi8(-(root2))");
    asm("lpm r28, Z");
    asm("ldi r29, 0x00");
    asm("lds r30, blip_brightaccum1 + 0");
    asm("lds r31, blip_brightaccum1 + 1");
    asm("add r30, r28");
  }
  

  // send 80 cycle pulse
  {
    asm("lds r25, blip_bits_blue + 5");
    asm("out %0, r25" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  // 77 cycle gap

  // (part 2 of 2, this part is 5 cycles)
  {		
    asm("adc r31, r29");
    asm("sts blip_brightaccum1 + 0, r30");
    asm("sts blip_brightaccum1 + 1, r31");
  }
  
  // 14 cycles
  {
    // uint16_t temp = blip_bright2;
    // if(blip_sample2 >= blip_trigger2)
    asm("lds r28, blip_sample2 + 0");
    asm("lds r29, blip_sample2 + 1");
    asm("lds r30, blip_trigger2 + 0");
    asm("lds r31, blip_trigger2 + 1");
    asm("cp r28, r30");
    asm("cpc r29, r31");
    asm("lds r28, blip_bright2 + 0");
    asm("lds r29, blip_bright2 + 1");
  }

  // 12 cycles. don't split this block.
  {
    // if(blip_sample2 >= blip_trigger2)
    //   temp = (temp <= (65535 - BRIGHT2_UP)) ? temp + BRIGHT2_UP : 65535;
    // else
    //   temp = (temp >= BRIGHT2_DOWN) ? temp - BRIGHT2_DOWN : 0;
    
    asm("brcs signal_low2");
    asm("nop");
  
    asm("signal_high2:");
    asm("ldi r30, %0" : : "M"(lo8(65535 - BRIGHT2_UP)) );
    asm("ldi r31, %0" : : "M"(hi8(65535 - BRIGHT2_UP)) );
    asm("cp r28, r30");
    asm("cpc r29, r31");
    asm("brcc bright_max2");
    asm("subi r28, %0" : : "M"(lo8(-BRIGHT2_UP)) );
    asm("sbci r29, %0" : : "M"(hi8(-BRIGHT2_UP)) );
    asm("nop");
    asm("rjmp store_bright2");

    asm("signal_low2:");
    asm("ldi r30, %0" : : "M"(lo8(BRIGHT2_DOWN)) );
    asm("ldi r31, %0" : : "M"(hi8(BRIGHT2_DOWN)) );
    asm("cp r28, r30");
    asm("cpc r29, r31");
    asm("brcs bright_min2");
    asm("subi r28, %0" : : "M"(lo8(BRIGHT2_DOWN)) );
    asm("sbci r29, %0" : : "M"(hi8(BRIGHT2_DOWN)) );
    asm("nop");
    asm("rjmp store_bright2");

    asm("bright_max2:");
    asm("ser r28");
    asm("ser r29");
    asm("rjmp store_bright2");
  
    asm("bright_min2:");
    asm("clr r28");
    asm("clr r29");
    asm("rjmp store_bright2");
  }

  // 4 cycles
  {
    // blip_bright2 = temp;
    asm("store_bright2:");
    asm("sts blip_bright2 + 0, r28");
    asm("sts blip_bright2 + 1, r29");
  }
  
  // 9 cycles, leaves 'bright2' in r30.
  {
    // bright2 = pgm_read_byte(exptab+(blip_bright2 >> 8));
    asm("mov r30, r29");
    asm("ldi r31, 0x00");
    asm("subi r30, lo8(-(exptab))");
    asm("sbci r31, hi8(-(exptab))");
    asm("lpm r30, Z");
    asm("nop");
    asm("nop");
  }

  // 17 cycles
  {
    // blip_brightaccum2 += pgm_read_byte(root2+bright2);
    asm("ldi r31, 0x00");
    asm("subi r30, lo8(-(root2))");
    asm("sbci r31, hi8(-(root2))");
    asm("lpm r28, Z");
    asm("ldi r29, 0x00");
    asm("lds r30, blip_brightaccum2 + 0");
    asm("lds r31, blip_brightaccum2 + 1");
    asm("add r30, r28");
    asm("adc r31, r29");
    asm("sts blip_brightaccum2 + 0, r30");
    asm("sts blip_brightaccum2 + 1, r31");
  }

  // restore temp registers
  asm("pop r29");
  asm("pop r28");
  // can't restore r25 here, we're out of cycles - do it below.
  // (and after we've updated the button...)

  // set next callback
  asm("ldi r30, pm_lo8(blue_field_B)");
  asm("ldi r31, pm_hi8(blue_field_B)");
  asm("sts blip_timer_callback, r30");
  asm("sts blip_timer_callback+1, r31");
  
  // set next timeout
  asm("ldi r30, %0" : : "M" (lo8(BLUE_FIELD_A_TIMEOUT)) );
  asm("ldi r31, %0" : : "M" (hi8(BLUE_FIELD_A_TIMEOUT)) );
  asm("sts %0, r31" : : "M" (&TCNT1H));
  asm("sts %0, r30" : : "M" (&TCNT1L));
  
  // send 160 cycle pulse
  {
    asm("lds r30, blip_bits_blue + 6");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }
  
  // Update button state. 27 cycles + 8 cycles call overhead.
  // Safe to call this from here as it uses r25, r30, r31
  asm("call UpdateButtons");

  // restore r25 out here because the 80 cycle block is completely packed.
  asm("pop r25");

  // Restore status register, R31, and R30.
  asm("pop r30");
  asm("out 0x3f, r30");
  asm("pop r31");
  asm("pop r30");

  // Done
  asm("reti");
}	
};
  
//---------------------------------------------------------------------------------
// Blue field B. Note that this callback is also responsible for triggering the
// next ADC blip_sample1 conversion, so that it will happen during the relatively
// 'quiet' period between PWM periods. We also set the 'vblank' flag here so
// that clients that care about synchronizing with the PWM frequency can do so.

extern "C" {
__attribute__((naked, aligned(4))) void blue_field_B() {
  // jitter padding
  asm("nop");
  asm("nop");  
  asm("nop");  

  // set next callback
  asm("ldi r30, pm_lo8(red_field_A)");
  asm("ldi r31, pm_hi8(red_field_A)");
  asm("sts blip_timer_callback, r30");
  asm("sts blip_timer_callback+1, r31");

  // Set next interrupt timeout. 6 cycles.
  asm("ldi r30, %0" : : "M" (lo8(BLUE_FIELD_B_TIMEOUT)) );
  asm("ldi r31, %0" : : "M" (hi8(BLUE_FIELD_B_TIMEOUT)) );
  asm("sts %0, r31" : : "M" (&TCNT1H));
  asm("sts %0, r30" : : "M" (&TCNT1L));
  
  // send 320 cycle pulse
  {
    asm("lds r30, blip_bits_blue + 7");
    asm("out %0, r30" : : "I"(_SFR_IO_ADDR(PORT_SOURCE)) );
  }		
  
  // We are now outside the hard-real-time section of the interrupt handler.
  // Code below this point should still run in a mostly-constant amount of time, but
  // it does not have to.
  
  // We've displayed all the fields for this PWM cycle, signal to the
  // application that it is now safe to swap frames. (3 cycles)
  asm("ldi r30, 1");
  asm("sts blip_blank, r30");
  
  // Skip updating the audio sample if audio is disabled due to the main app
  // needing to use the ADC to check the battery voltage or something.
  
  asm("lds r30, blip_audio_enable");
  asm("tst r30");
  asm("breq blip_no_audio");
  
  // Store the previous ADC sample
  asm("lds r30, %0" : : "M" (&ADCL) );
  asm("sts blip_sample + 0, r30");
  asm("lds r30, %0" : : "M" (&ADCH) );
  asm("sts blip_sample + 1, r30");
  
  // set ADC start conversion flag
  asm("lds r30, %0" : : "M" (&ADCSRA) );
  asm("ori r30, %0" : : "M" (bit(ADSC)) );
  asm("sts %0, r30" : : "M" (&ADCSRA) );
  
  asm("blip_no_audio:");

  // Restore status register, R31, and R30.
  asm("pop r30");
  asm("out 0x3f, r30");
  asm("pop r31");
  asm("pop r30");

  // Done
  asm("reti");
}
};

//---------------------------------------------------------------------------------
// Update button state. 27 cycles + call overhead. Uses r25, r30, r31
// This is separate from the LED interrupt handlers as we also need to be able
// to call it from our sleep mode watchdog interrupt.

extern "C" {
__attribute__((naked)) void UpdateButtons()
{
  asm("lds r25, buttonstate1");
  asm("sbis %0, %1" : : "I"(_SFR_IO_ADDR(PINC)), "M"(BUTTON1_PIN));
  asm("jmp button_down1");

  asm("button_up1:");
  asm("lds r30, debounce_up1 + 0");
  asm("lds r31, debounce_up1 + 1");
  
  // if(buttonstate1 == 0) debounce_up1 = 0;
  asm("sbrs r25, 0");
  asm("clr r30");
  asm("sbrs r25, 0");
  asm("clr r31");

  // if(debounce_up1 < 0x8000) debounce_up1++;
  asm("sbrs r31, 7");
  asm("subi r30, 0xFF");
  asm("sbrs r31, 7");
  asm("sbci r31, 0xFF");

  // store buttonstate1 & debounce_up1
  asm("ldi r25, 1");
  asm("sts buttonstate1, r25");
  asm("sts debounce_up1 + 0, r30");
  asm("sts debounce_up1 + 1, r31");

  asm("jmp button_done1");

  asm("button_down1:");
  asm("lds r30, debounce_down1 + 0");
  asm("lds r31, debounce_down1 + 1");
  
  // if(buttonstate1 == 1) debounce_down1 = 0;
  asm("sbrc r25, 0");
  asm("clr r30");
  asm("sbrc r25, 0");
  asm("clr r31");

  // if(debounce_down1 < 0x8000) debounce_down1++;
  asm("sbrs r31, 7");
  asm("subi r30, 0xFF");
  asm("sbrs r31, 7");
  asm("sbci r31, 0xFF");

  // store buttonstate1 & debounce_up1
  asm("clr r25");
  asm("sts buttonstate1, r25");
  asm("sts debounce_down1 + 0, r30");
  asm("sts debounce_down1 + 1, r31");

  asm("nop");
  asm("nop");

  asm("button_done1:");

  asm("lds r25, buttonstate2");
  asm("sbis %0, %1" : : "I"(_SFR_IO_ADDR(PINC)), "M"(BUTTON2_PIN));
  asm("jmp button_down2");

  asm("button_up2:");
  asm("lds r30, debounce_up2 + 0");
  asm("lds r31, debounce_up2 + 1");
  
  // if(buttonstate2 == 0) debounce_up2 = 0;
  asm("sbrs r25, 0");
  asm("clr r30");
  asm("sbrs r25, 0");
  asm("clr r31");

  // if(debounce_up2 < 0x8000) debounce_up2++;
  asm("sbrs r31, 7");
  asm("subi r30, 0xFF");
  asm("sbrs r31, 7");
  asm("sbci r31, 0xFF");

  // store buttonstate2 & debounce_up2
  asm("ldi r25, 1");
  asm("sts buttonstate2, r25");
  asm("sts debounce_up2 + 0, r30");
  asm("sts debounce_up2 + 1, r31");

  asm("jmp button_done2");

  asm("button_down2:");
  asm("lds r30, debounce_down2 + 0");
  asm("lds r31, debounce_down2 + 1");
  
  // if(buttonstate2 == 1) debounce_down2 = 0;
  asm("sbrc r25, 0");
  asm("clr r30");
  asm("sbrc r25, 0");
  asm("clr r31");

  // if(debounce_down2 < 0x8000) debounce_down2++;
  asm("sbrs r31, 7");
  asm("subi r30, 0xFF");
  asm("sbrs r31, 7");
  asm("sbci r31, 0xFF");

  // store buttonstate2 & debounce_up2
  asm("clr r25");
  asm("sts buttonstate2, r25");
  asm("sts debounce_down2 + 0, r30");
  asm("sts debounce_down2 + 1, r31");

  asm("nop");
  asm("nop");

  asm("button_done2:");

  asm("ret");
}
};

//---------------------------------------------------------------------------------
// Converts our 8-pixel framebuffer from rgb values to bit planes. If you think
// of each color channel as being an 8x8 matrix of bits, this is basically a
// transpose of that matrix.

// The compiler generates incredibly dumb code for this - the assembly version
// is almost twice as fast.

// Permute and transpose an 8x8 matrix of bits.

__attribute__((naked)) void swap4d(void* vin, uint8_t* vout) {
  asm("push r0");
  asm("push r1");

  asm("movw r30, r24");
  asm("movw r26, r22");
  
  // Our blip_pixels are in logical order but our LEDs may be in some other order
  // due to routing constraints, so we reshuffle them using the PIN_TO_PIXEL
  // constants as we load them. Note the "*3" so that we pick up values from
  // the same color channel.

  asm("ldd r18, z+%0*6 + 1" : : "M"(PIN_0_TO_PIXEL));
  asm("ldd r19, z+%0*6 + 1" : : "M"(PIN_1_TO_PIXEL));
  asm("ldd r20, z+%0*6 + 1" : : "M"(PIN_2_TO_PIXEL));
  asm("ldd r21, z+%0*6 + 1" : : "M"(PIN_3_TO_PIXEL));
  
  asm("ldd r22, z+%0*6 + 1" : : "M"(PIN_4_TO_PIXEL));
  asm("ldd r23, z+%0*6 + 1" : : "M"(PIN_5_TO_PIXEL));
  asm("ldd r24, z+%0*6 + 1" : : "M"(PIN_6_TO_PIXEL));
  asm("ldd r25, z+%0*6 + 1" : : "M"(PIN_7_TO_PIXEL));
  
  // Gamma-correct all the values.
  asm("mul r18, r18"); asm("mov r18, r1");
  asm("mul r19, r19"); asm("mov r19, r1");
  asm("mul r20, r20"); asm("mov r20, r1");
  asm("mul r21, r21"); asm("mov r21, r1");
  asm("mul r22, r22"); asm("mov r22, r1");
  asm("mul r23, r23"); asm("mov r23, r1");
  asm("mul r24, r24"); asm("mov r24, r1");
  asm("mul r25, r25"); asm("mov r25, r1");
  
  // Done with r30:r31, they now become our loop counter and
  // bit collector.
  asm("ldi r30, 8");

  asm("swaploop2:");
  // Skim the low bits off all the temp registers and collect them in R31.
  asm("lsr r18"); asm("ror r31");
  asm("lsr r19"); asm("ror r31");
  asm("lsr r20"); asm("ror r31");
  asm("lsr r21"); asm("ror r31");
  asm("lsr r22"); asm("ror r31");
  asm("lsr r23"); asm("ror r31");
  asm("lsr r24"); asm("ror r31");
  asm("lsr r25"); asm("ror r31");

  // Store r31 to the front buffer.
  asm("st x+, r31");
  
  // Repeat 8 times.
  asm("dec r30");
  asm("brne swaploop2");

  asm("pop r1");
  asm("pop r0");
  asm("ret");
}

//------------------------------------------------------------------------------
// Synchronize with our PWM interrupt, and then swap framebuffers. The swap is
// not a simple copy, we actually swizzle the bits in the framebuffer to
// convert from logical to physical pixel order & from brightness values to bit
// plane values.

// We also do userland bookkeeping here for things that don't need to be tackled
// every PWM cycle - checking for idle mode, checking battery voltage, etc.

void blip_swap() {
  // If we hear bass, remember when we heard it.
  if (blip_bright2 > 32768) {
    blip_idle = blip_tick;
  }
  // If we haven't heard any bass in 10 minutes, go to sleep.
  if ((blip_tick - blip_idle) > 2457600) {
    blip_sleep();
  }
  
  // TODO(aappleby): Check battery voltage every N seconds & go to sleep if low.
  
  // Wait for the blanking flag to go from low to high.
  while(blip_blank);
  while(!blip_blank);
  
  // We're now in between periods, convert our framebuffer to bit-plane
  // format and store it in the front buffer.
  swap4d(&blip_pixels[0].r, blip_bits_red);
  swap4d(&blip_pixels[0].g, blip_bits_green);
  swap4d(&blip_pixels[0].b, blip_bits_blue);
}


//------------------------------------------------------------------------------
// Swap framebuffers on a multiple of 64 ticks, which synchronizes us at 64 hz.

void blip_swap64() {
  while (blip_tick & 63) {};
  blip_swap();
}  

//------------------------------------------------------------------------------
// Trivial framebuffer clear, because GCC is dumb.

__attribute__((naked)) void blip_clear() {
  asm("sts blip_pixels +  0, r1"); asm("sts blip_pixels +  1, r1"); asm("sts blip_pixels +  2, r1");
  asm("sts blip_pixels +  3, r1"); asm("sts blip_pixels +  4, r1"); asm("sts blip_pixels +  5, r1");
  asm("sts blip_pixels +  6, r1"); asm("sts blip_pixels +  7, r1"); asm("sts blip_pixels +  8, r1");
  asm("sts blip_pixels +  9, r1"); asm("sts blip_pixels + 10, r1"); asm("sts blip_pixels + 11, r1");
  asm("sts blip_pixels + 12, r1"); asm("sts blip_pixels + 13, r1"); asm("sts blip_pixels + 14, r1");
  asm("sts blip_pixels + 15, r1"); asm("sts blip_pixels + 16, r1"); asm("sts blip_pixels + 17, r1");
  asm("sts blip_pixels + 18, r1"); asm("sts blip_pixels + 19, r1"); asm("sts blip_pixels + 20, r1");
  asm("sts blip_pixels + 21, r1"); asm("sts blip_pixels + 22, r1"); asm("sts blip_pixels + 23, r1");
  asm("sts blip_pixels + 24, r1"); asm("sts blip_pixels + 25, r1"); asm("sts blip_pixels + 26, r1");
  asm("sts blip_pixels + 27, r1"); asm("sts blip_pixels + 28, r1"); asm("sts blip_pixels + 29, r1");
  asm("sts blip_pixels + 30, r1"); asm("sts blip_pixels + 31, r1"); asm("sts blip_pixels + 32, r1");
  asm("sts blip_pixels + 33, r1"); asm("sts blip_pixels + 34, r1"); asm("sts blip_pixels + 35, r1");
  asm("sts blip_pixels + 36, r1"); asm("sts blip_pixels + 37, r1"); asm("sts blip_pixels + 38, r1");
  asm("sts blip_pixels + 39, r1"); asm("sts blip_pixels + 40, r1"); asm("sts blip_pixels + 41, r1");
  asm("sts blip_pixels + 42, r1"); asm("sts blip_pixels + 43, r1"); asm("sts blip_pixels + 44, r1");
  asm("sts blip_pixels + 45, r1"); asm("sts blip_pixels + 46, r1"); asm("sts blip_pixels + 47, r1");
  asm("ret");
}

//------------------------------------------------------------------------------
// Minimalist LED test, just verifies that each LED can be addressed
// independently.

void delayblah() {
  volatile uint8_t x = 1;
  for (int i = 0; i < 30000; i++) {
    x *= 123;
    x ^= x >> 4;
    x *= 123;
    x ^= x >> 4;
  }    
}  

void blip_selftest() {
  // Turn the watchdog timer off.
  MCUSR &= ~(1 << WDRF);
  WDTCSR = bit(WDCE) | bit(WDE);
  WDTCSR = 0;

  // Turn off the serial interface, which the Arduino bootloader leaves on
  // by default.
#ifdef BOARDTYPE_TEENSY2
  UCSR1B &= ~(1 << RXEN1);
  UCSR1B &= ~(1 << TXEN1);
#else
  UCSR0B &= ~(1 << RXEN0);
  UCSR0B &= ~(1 << TXEN0);
#endif
  
  DIR_SINK = 0xFF;
  PORT_SINK = 0x00;
  
  DIR_SOURCE = 0xFF;
  PORT_SOURCE = 0x01;
  
  while(1)
  {
    PORT_SINK = SINK_RED;
    PORT_SOURCE = 1 << PIXEL_0_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_1_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_2_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_3_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_4_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_5_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_6_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_7_TO_PIN; delayblah();
    
    PORT_SINK = SINK_GREEN;
    PORT_SOURCE = 1 << PIXEL_0_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_1_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_2_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_3_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_4_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_5_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_6_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_7_TO_PIN; delayblah();

    PORT_SINK = SINK_BLUE;
    PORT_SOURCE = 1 << PIXEL_0_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_1_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_2_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_3_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_4_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_5_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_6_TO_PIN; delayblah();
    PORT_SOURCE = 1 << PIXEL_7_TO_PIN; delayblah();
  }
}

//---------------------------------------------------------------------------------
// Turns off every peripheral in the ATMega.

void blip_shutdown() {
  // Set all ports to input mode.
  
  DIR_SINK = 0x00;
  DIR_STATUS = 0x00;
  DIR_SOURCE = 0x00;
  
  // Disable pullups on all ports except for our two button pins so we can come
  // out of sleep mode
  PORT_SINK = 0x00;
  PORT_STATUS = (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN);
  PORT_SOURCE = 0x00;
  
  // Disable AD converter.
  ADCSRA = 0;
  ADCSRB = 0;
  ADMUX = 0x0F; // ref = aref, left adjust = false, mux = ground
  
  // Disable analog comparator
  ACSR = bit(ACD);

  // TODO(aappleby): er, we should probably be turning off the digital input
  // buffer for the analog pins here, right?
  // DIDR0 = 0x00;  // disable digital input for all analog inputs
  // DIDR1 = 0x03;  // disable digital input for all comparator inputs
  DIDR0 = 0xFF & ~((1 << BUTTON1_PIN) | (1 << BUTTON2_PIN));
  DIDR1 = 0xFF;
  
  // Disable all external interrupts.
  EICRA = 0;
  EIMSK = 0;
  PCICR = 0;
  PCMSK0 = 0;
  PCMSK1 = 0;
  PCMSK2 = 0;
  
  // Turn off the SPI interface.
  SPCR = 0;
  
  // Turn off the serial interface, which the bootloader leaves on by default.
#ifdef BOARDTYPE_TEENSY2
  UCSR1A = 0;
  UCSR1B = 0;
#else
  UCSR0A = 0;
  UCSR0B = 0;
#endif
  
  // Disable TWI
  TWCR = 0;
  
  // Disable timer 0
  TIMSK0 = 0;
  TCCR0A = 0;
  TCCR0B = 0;

  // Disable timer 1
  TIMSK1 = 0;
  TCCR1A = 0;
  TCCR1B = 0;
  
  // Disable timer 2
  TIMSK2 = 0;
  TCCR2A = 0;
  TCCR2B = 0;
  
  // Disable watchdog
  MCUSR &= ~(1 << WDRF);
  WDTCSR = bit(WDCE) | bit(WDE);
  WDTCSR = 0;
  
  // Power down all peripherals. This has to be done last otherwise some of the
  // above settings don't actually do anything.
#ifdef BOARDTYPE_TEENSY2
  //PRR0 = bit(PRTWI) | bit(PRTIM2) | bit(PRTIM0) | bit(PRTIM1) | bit(PRSPI) | bit(PRADC);
  //PRR1 = bit(PRUSB) | bit(PRTIM4) | bit(PRTIM3) | bit(PRUSART1);
  PRR0 = 0xFF;
  PRR1 = 0xFF;
#else
  PRR = bit(PRTWI) | bit(PRTIM0) | bit(PRTIM1) | bit(PRTIM2) | bit(PRSPI) | bit(PRUSART0) | bit(PRADC);
#endif
}

//---------------------------------------------------------------------------------
// Initial configuration of all peripherals - we set up the LED array, turn on
// the microphone, enable pull-ups for our buttons, start the ADC, configure our
// timer interrupts, and kick off the first ADC blip_sample1.

void blip_setup() {
  // No firing interrupts while we're configuring things.
  cli();
  
  // If we're running off a 16 mhz crystal, divide the master clock by 2 to get
  // 8 mhz.
#ifdef USING_CRYSTAL
  CLKPR = 1 << CLKPCE;
  CLKPR = 1 << CLKPS0;
#endif
  
  // Turn absolutely everything off to start with.
  blip_shutdown();
  
  // Port D is our LED source port.	
  PORT_SOURCE = 0x00;
  DIR_SOURCE = 0xFF;

  // Port B is our LED sink port.
  PORT_SINK = 0x00;
  DIR_SINK = 0xFF;

  // Port C is our button input, mic input, and status port. We also power the
  // microphone from this port so that we can turn it off during sleep mode.
  DIR_STATUS = (1 << MIC_POWER);
  PORT_STATUS = (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << MIC_POWER);
  
  // Disable sleep modes.
  SMCR = 0;
  
  // Turn the watchdog timer off.
  MCUSR &= ~(1 << WDRF);
  WDTCSR = bit(WDCE) | bit(WDE);
  WDTCSR = 0;

  // Turn the ADC on and set it up to read from the microphone input,
  // left-adjust the result, and blip_sample1 in (14 * 32) = 448 cycles (56 uS
  // at 8 mhz) - the fastest we can blip_sample1 and still get 10-bit resolution.

#ifdef BOARDTYPE_TEENSY2
  PRR0 &= ~bit(PRADC);
#else
  PRR &= ~bit(PRADC);
#endif

  ADMUX  = MIC_PIN | bit(ADLAR);
  ADCSRA = bit(ADEN) | bit(ADPS2) | bit(ADPS0);
  //DIDR0 = 0x3F & ~((1 << BUTTON1_PIN) | (1 << BUTTON2_PIN));
  //DIDR1 = 0xFF;

  // Set timer 1 to tick at full speed and generate overflow interrupts.
#ifdef BOARDTYPE_TEENSY2
  PRR0 &= ~bit(PRTIM1);
#else
  PRR &= ~bit(PRTIM1);
#endif
  TIMSK1 = (1 << TOIE1);
  TCCR1A = 0;
  TCCR1B = (1 << CS10);

  // Hardware peripherals are configured, reset all the software state.
  
  // Reset the global timers.
  blip_tick = 0;
  blip_idle = 0;
  
  // Wait for all buttons to be released & reset button state.
  //while((PINC & ((1 << BUTTON1_PIN) | (1 << BUTTON2_PIN))) == 0);
  
  buttonstate1 = 0;
  debounce_up1 = 0;
  debounce_down1 = 0;

  buttonstate2 = 0;
  debounce_up2 = 0;
  debounce_down2 = 0; 

  // The first callback that our timer interrupt will fire is for the first
  // half of the red field.
  blip_timer_callback = red_field_A;
    
  // Device configured, kick off the first ADC conversion and enable
  // interrupts.
  blip_audio_enable = 1;
  sbi(ADCSRA,ADSC);
  sei();
}

//---------------------------------------------------------------------------------
// Sleep mode support.

// The watchdog interrupt handler has to exist or the chip resets.
ISR(WDT_vect) {}

// We want the breathing LED effect to go back and forth across the LEDs. This table
// maps which logical LED we want to make breathe to the physical pin we need to turn
// on for that LED.
uint8_t extern const PROGMEM sources[]  =
{
  1 << PIXEL_0_TO_PIN,
  1 << PIXEL_1_TO_PIN,
  1 << PIXEL_2_TO_PIN,
  1 << PIXEL_3_TO_PIN,
  1 << PIXEL_4_TO_PIN,
  1 << PIXEL_5_TO_PIN,
  1 << PIXEL_6_TO_PIN,
  1 << PIXEL_7_TO_PIN,
  1 << PIXEL_6_TO_PIN,
  1 << PIXEL_5_TO_PIN,
  1 << PIXEL_4_TO_PIN,
  1 << PIXEL_3_TO_PIN,
  1 << PIXEL_2_TO_PIN,
  1 << PIXEL_1_TO_PIN,
  1 << PIXEL_0_TO_PIN,
};

extern const uint8_t sintab[] PROGMEM;

void blip_sleep()
{
  // Turn off all peripherals and disable brownout detection during sleep mode.
  blip_shutdown();
  //sleep_bod_disable();
  
  sei();

  // Enable sleep mode.
  SMCR = bit(SE) | bit(SM1);

  // Turn on the watchdog timer.
  WDTCSR = bit(WDCE) | bit(WDE);
  WDTCSR = bit(WDIE);

  // Set the LED array up to display only a single color channel.
  DIR_SOURCE = 0xFF;
  PORT_SOURCE = 0x00;
  DIR_SINK = 0xFF;
  PORT_SINK = SINK_GREEN;
  
  uint8_t sin_cursor = 128;
  uint8_t led_cursor = 0;
  uint8_t button_counter = 0;
  uint8_t sink_cursor = 0;
  
  uint8_t sinks[] = { SINK_RED, SINK_GREEN, SINK_BLUE };

  // Sleep forever (or until the user presses a button).
  while(1)
  {
    // Go to sleep. After 16 milliseconds the watchdog timer will wake us up.
    asm("sleep");
    
    // When we wake up, update our buttons.
    button_counter++;
    if (PINC & (1 << BUTTON1_PIN)) {
      button_counter = 0;
    }      
    
    // If button 1 has been pressed for 7 ticks (~1/8 second), turn off the watchdog
    // timer and leave sleep mode.
    if(button_counter == 7) break;

    // Otherwise send a tiny pulse of light out through one of the green LEDs.
    // The length of the pulse is determined by the contents of the sine wave
    // table, which gives us a nice breathing effect.
    uint8_t bright = pgm_read_byte(sintab + uint8_t(sin_cursor - 64));
    // Gamma-correct the sine wave and reduce its brightness to 25%.
    bright = (bright * bright) >> 8;
    bright = bright >> 2;
    
    // If the brightness is greater than zero, turn one green pixel on for a
    // time proportional to the brightness.
    if(bright)
    {
      PORT_SOURCE = pgm_read_byte(sources + led_cursor);
      while(bright--) asm("nop");
      PORT_SOURCE = 0;
    }
    
    // Once we've finished a full breathing cycle, step to the next LED.
    sin_cursor++;
    if(sin_cursor == 0) {
      led_cursor++;
      if (led_cursor == 14) led_cursor = 0;
      sink_cursor++;
      if (sink_cursor == 3) sink_cursor = 0;
      PORT_SINK = sinks[sink_cursor];
    }      
  }

  // We're waking up, reinitialize all the peripherals.
  blip_setup();
}

//---------------------------------------------------------------------------------

int blip_button1_held(uint16_t ticks) {
  return ((buttonstate1 == 0) && (debounce_down1 >= ticks)) ? 1 : 0;
}

int blip_button1_released_after(uint16_t ticks) {
  return ((buttonstate1 == 1) && (debounce_down1 >= ticks)) ? 1 : 0;
}  

int blip_button2_held(uint16_t ticks) {
  return ((buttonstate2 == 0) && (debounce_down2 >= ticks)) ? 1 : 0;
}

int blip_button2_released_after(uint16_t ticks) {
  return ((buttonstate2 == 1) && (debounce_down2 >= ticks)) ? 1 : 0;
}  

//-----------------------------------------------------------------------------
