#include "LEDDriver.h"

void setup() {
  BliplaceSetup();
}

void loop() {
  for(int i = 0; i < 8; i++) {
    pixels[i].red = treble;
    pixels[i].blue = bass;
  }
  
  BliplaceSwap();
}
