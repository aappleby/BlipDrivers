#include "Sleep.h"

#include "Config.h"
#include "LEDDriver.h"
#include "Bits.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

extern "C"
{
  void ShutStuffDown();
  void UpdateButtons();
};

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

extern const uint8_t gammasin[] PROGMEM;

// watchdog interrupt has to exist or the chip resets.
ISR(WDT_vect) {}

void GoToSleep()
{
  uint8_t cursor = 0;
  uint8_t column = 0;

  ShutStuffDown();
  sleep_bod_disable();

  SMCR = bit(SE) | bit(SM1);
  WDTCSR = bit(WDCE) | bit(WDE);
  WDTCSR = bit(WDIE);
  
  DDRD = 0xFF;
  PORTD = 0x00;
  DDRB = 0xFF;
  PORTB = SINK_GREEN;
  
  while(1)
  {
    asm("sleep");
    
    UpdateButtons();
    
    if((buttonstate1 == 0) && (debounce_down1 > 60))
    {
      SMCR = 0;
      WDTCSR = bit(WDCE) | bit(WDE);
      WDTCSR = 0;
      SetupLEDs();
      return;
    }

    uint8_t bright = pgm_read_byte(gammasin + cursor) >> 2;
    if(bright)
    {
      PORTD = pgm_read_byte(sources + column);
      for(uint8_t i = 0; i < bright; i++) { asm("nop"); }
      PORTD = 0;
    }
    
    cursor += 1;
    if(cursor == 0) column = (column == 13) ? 0 : column + 1;
  }
}

void FadeOutAndSleep()
{
  Pixel backup[8];
  
  uint32_t old_tick = blip_tick;
  for(int i = 0; i < 8; i++)
  {
    backup[i] = pixels[i];
  }
  for(uint16_t i = 0; i < 256; i++) {
    uint8_t f = ((255-i) * (255-i)) >> 8;
    for(int j = 0; j < 8; j++) {
      pixels[j].r = (backup[j].r * f) >> 8;
      pixels[j].g = (backup[j].g * f) >> 8;
      pixels[j].b = (backup[j].b * f) >> 8;
    }
    for(int j = 0; j < 13; j++)
    {
      blip_swap();
    }
  }
  // wait for button release before going to sleep
  while(buttonstate1 == 0);
  GoToSleep();
  
  for(uint16_t i = 0; i < 256; i++)
  {
    uint8_t f = (i * i) >> 8;
    for(int j = 0; j < 8; j++)
    {
      pixels[j].r = (backup[j].r * f) >> 8;
      pixels[j].g = (backup[j].g * f) >> 8;
      pixels[j].b = (backup[j].b * f) >> 8;
    }
    for(int j = 0; j < 13; j++)
    {
      blip_swap();
    }
  }
  blip_tick = old_tick;
  while(buttonstate1 == 0);
  debounce_down1 = 0;
  debounce_up1 = 0;
}

void UpdateSleep()
{
  if((buttonstate1 == 0) && (debounce_down1 > 16384)) {
    FadeOutAndSleep();
  }
}