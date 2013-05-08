// tables.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <memory.h>

typedef unsigned char uint8_t;
typedef signed char int8_t;
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

    double a = (numbers[j + 0] + numbers[j + 1]) / 2;
    double b = numbers[j + 1];
    double c = (numbers[j + 1] + numbers[j + 2]) / 2;

    for(int i = 0; i < 20; i++) {
      double t = double(i) / 20.0;

      double e = a + (b-a) * t;
      double f = b + (c-b) * t;
      double g = e + (f-e) * t;

      printf("%f\n", g);
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

void tabprint8(uint8_t d, int i) {
  char buf[16];
  sprintf(buf, "%3d", d);
  printf("%s, ", buf);
  if(i % 16 == 15) {
    printf("\n");
  }
}

void tabprint_s8(int8_t d, int i) {
  char buf[16];
  sprintf(buf, "%4d", d);
  printf("%s, ", buf);
  if(i % 16 == 15) {
    printf("\n");
  }
}

void tabprint16(uint16_t d, int i) {
  char buf[16];
  sprintf(buf, "%5d", d);
  printf("%s, ", buf);
  if(i % 16 == 15) {
    printf("\n");
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
    tabprint8(x, i);
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

double lum_to_cie(double l) {
  if(l <= 0.08) {
    return (100.0 * l / 902.3);
  } else  {
    float y = (100.0 * l + 16.0) / 116.0;
    y = y*y*y;
    return y;
  }
}

double cie_to_lum(double y) {
  if (y < 0.008856) {
    return (903.3 * y) / 100.0;
  } else {
    return (116 * pow(y, 1.0 / 3.0) - 16) / 100.0;
  }
}

/*
void huetab() {
  char buf[256];
  for(int i = 0; i < 256; i++) {
    float s = 6.0 * float(i) / 256.0;
    int h = (int)s;
    float t = fmod(s, 1.0f);
    switch(h) {
    case 0: break;
    case 1: t = 1.0; break;
    case 2: t = 1.0; break;
    case 3: t = 1.0 - t; break;
    case 4: t = 0; break;
    case 5: t = 0; break;
    };
    tabprint8(floor(t * 255.0 - 0.5));
  }
}
*/

void s1p14_to_cie() {
  for(int i = 0; i <= 256; i++) {
    float t = float(i) / 256.0;
    t = 4.0 * t - 2.0;

    if(t < 0) {
      tabprint16(0, i);
    } else if (t > 1) {
      tabprint16(65535, i);
    } else {
      double l = floor(lum_to_cie(t) * 65535.0 + 0.5);
      tabprint16(l, i);
    }
  }
}


void ssintab() {
  for(int i = 0; i < 256; i++) {
    double t = (i / 256.0) * pi * 2.0;
    double y = sin(t) * 127;
    tabprint_s8(y, i);
  }
}


int main(int argc, char** argv)
{
  for(int i = 0; i < 257; i++) {
    tabprint8(i, i);
  }

  return 0;
}

