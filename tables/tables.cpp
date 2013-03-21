// tables.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <math.h>
#include <assert.h>

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

double pi = 3.14159265358979323846264338327950288419716939937510;

double numbers[] = {
63,82,95,59,72,24,58,78,26,80,70,18,23,84,55,39,17,50,59,63,44,87,22,
100,85,37,95,80,27,96,33,45,29,24,41,27,74,24,30,78,86,22,27,9,4,36,
97,72,11,33,98,70,42,74,11,3,55,16,84,55,97,34,39,11,24,8,44,23,45,7,
48,80,30,39,33,4,86,87,7,87,71,92,67,38,38,67,24,66,16,55,50,46,71,75,
28,100,50,33,63,39
};

void quadlerp() {

  for(int j = 0; j < 10; j++) {

    double a = numbers[j + 1];
    double b = (numbers[j + 1] + numbers[j + 3]) / 2;
    double c = (numbers[j + 0] + numbers[j + 2]) / 2;
    double d = numbers[j + 2];

    for(int i = 0; i < 20; i++) {
      double t = double(i) / 20.0;

      double f = ((a - b - c + d) * t * t) + (-2*a + b + c)*t + a;
      printf("%f\n", f);
    }
  }
}

void cubelerp() {

  for(int j = 0; j < 10; j++) {

    double p0 = numbers[j + 1];
    double p1 = numbers[j + 2];
    double m0 = (numbers[j + 2] - numbers[j + 0]) / 2;
    double m1 = (numbers[j + 3] - numbers[j + 1]) / 2;

    for(int i = 0; i < 20; i++) {
      double t = double(i) / 20.0;
      double t2 = t*t;
      double t3 = t*t*t;

      double p = (2*t3 - 3*t2 + 1)*p0 + (t3 - 2*t2 + t)*m0 + (-2*t3 + 3*t2)*p1 + (t3 - t2)*m1;

      printf("%f\n", p);
    }
  }
}



void pulsetab() {
  uint8_t tab[256];
  char buf[16];
  for(int i = 0; i < 256; i++) {
    double t = double(i) / 256.0;
    t = pow(t, 1.0/2.0);
    t *= pi;
    double s = sin(t);
    s = pow(s, 6);

    uint8_t x = floor(s * 255.0 + 0.5);
    tab[i] = x;
    sprintf(buf, "%3d", x);
    printf("%s, ", buf);
    if(i % 16 == 15) {
      printf("\n");
    }
  }

  for(int j = 0; j < 16; j++) {
    uint8_t cutoff = 16 * (15-j) + 8;
    for(int i = 0; i < 256; i += 4) {
      if(tab[i] < cutoff) {
        printf(".");
      } else {
        printf("#");
      }
    }
    printf("\n");
  }
}

uint8_t square0(uint8_t v) {
  uint32_t x = v;
  x = (x*x);
  x = x >> 8;
  return x;
}

uint8_t square1(uint8_t v) {
  uint32_t x = v;
  x = (x*x) + x;
  x = x >> 8;
  return x;
}

uint8_t square2(uint8_t v) {
  uint32_t x = v;
  x = (x*x) + x + x;
  x = x >> 8;
  return x;
}

uint8_t square3(uint8_t v) {
  float l = (v / 255.0) * 100.0;
  if(l <= 8) {
    float y = (l / 902.3);
    y *= 255.0;
    return (uint8_t)y;
  } else  {
    float y = (l + 16.0) / 116.0;
    y = y*y*y;
    y *= 255.0;
    return (uint8_t)y;
  }
}

uint8_t ciesin(uint8_t v) {
  float t = (v - 64.0) / 256.0;
  float s = sin(2 * 3.14159265358979323846264338327950288419716939937510 * t);
  float l = (s + 1) / 2;
  l *= 100.0;
  if(l <= 8) {
    float y = (l / 902.3);
    y *= 255.0;
    return (uint8_t)y;
  } else  {
    float y = (l + 16.0) / 116.0;
    y = y*y*y;
    y *= 255.0;
    return (uint8_t)y;
  }
}


int _tmain(int argc, _TCHAR* argv[])
{
  cubelerp();
  return 0;

  for(int i = 0; i < 256; i++) {
    for(int j = 0; j < 256; j++) {
      uint16_t a = 0;
      
      a = (i + j) << 8;
      a -= ((uint16_t)(i * j));

      uint32_t b = 0;

      b += (i << 8);
      b += (j << 8);
      b -= (uint16_t)(i * j);

      assert(a == b);
      //printf("%02x ", a);

    }
    //printf("\n");
  }
  return 0;

  for(int i = 0; i <= 255; i++) {
    if(i % 16 == 0) printf("\n");
    uint8_t a = square0(i);
    uint8_t b = square1(i);
    uint8_t c = square2(i);
    uint8_t d = square3(i);
    uint8_t e = ciesin(i);
    //printf("%3d, %3d, %3d, %3d\n", a, b, c, d);
    printf("%3d, ", e);
  }
	return 0;
}

