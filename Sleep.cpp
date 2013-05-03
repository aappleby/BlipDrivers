#include "Sleep.h"

#include "Config.h"
#include "Math.h"
#include "LEDDriver.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <util/delay.h>

extern "C"
{
  void blip_shutdown();
  void UpdateButtons();
};

extern const uint8_t sintab[] PROGMEM;

// watchdog interrupt has to exist or the chip resets.
ISR(WDT_vect) {
//  PORTD = 0x02;
}

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

void GoToSleep()
{
  blip_shutdown();
  sleep_bod_disable();
  
  sei();

  // Turn on the watchdog timer.
  SMCR = bit(SE) | bit(SM1);
  WDTCSR = bit(WDCE) | bit(WDE);
  WDTCSR = bit(WDIE);

  // Set the LED array up to display only the green channel.
  DDRD = 0xFF;
  PORTD = 0x00;
  DDRB = 0xFF;
  PORTB = SINK_GREEN;
  
  uint8_t sin_cursor = 0;
  uint8_t led_cursor = 0;

  // Sleep forever (or until the user presses a button).
  while(1)
  {
    // Go to sleep. After 16 milliseconds the watchdog timer will wake us up.
    asm("sleep");
    
    // When we wake up, update our buttons.
    UpdateButtons();
    
    // If button 1 has been pressed for 60 ticks (~1 second), turn off the watchdog
    // timer and leave sleep mode.
    if((buttonstate1 == 0) && (debounce_down1 > 60))
    {
      SMCR = 0;
      WDTCSR = bit(WDCE) | bit(WDE);
      WDTCSR = 0;
      blip_setup();
      return;
    }

    // Otherwise send a tiny pulse of light out through one of the green LEDs.
    // The length of the pulse is determined by the contents of the gamma-corrected
    // sine wave table, which gives us a nice breathing effect.
    uint8_t bright = pgm_read_byte(sintab + uint8_t(sin_cursor - 64));
    // Gamma-correct the sine wave and reduce its brightness to 25%.
    bright = (bright * bright) >> 8;
    bright = bright >> 2;
    
    if(bright)
    {
      // Turn one green pixel on for a time proportional to the brightness.
      PORTD = pgm_read_byte(sources + led_cursor);
      while(bright--) asm("nop");
      PORTD = 0;
    }
    
    // Once we've finished a full breathing cycle, step to the next LED.
    sin_cursor++;
    if(sin_cursor == 0) {
      led_cursor = (led_cursor == 13) ? 0 : led_cursor + 1;
    }      
  }
}

void FadeOutAndSleep()
{
  Color backup[8];
  
  uint32_t old_tick = blip_tick;
  for(int i = 0; i < 8; i++)
  {
    backup[i] = blip_pixels[i];
  }
  for(uint16_t i = 0; i < 256; i++) {
    uint8_t f = ((255-i) * (255-i)) >> 8;
    for(int j = 0; j < 8; j++) {
      blip_pixels[j].r = (backup[j].r >> 8) * f;
      blip_pixels[j].g = (backup[j].g >> 8) * f;
      blip_pixels[j].b = (backup[j].b >> 8) * f;
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
      blip_pixels[j].r = (backup[j].r >> 8) * f;
      blip_pixels[j].g = (backup[j].g >> 8) * f;
      blip_pixels[j].b = (backup[j].b >> 8) * f;
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