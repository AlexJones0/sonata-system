#ifndef LCD_H
#define LCD_H

#include "st7735/lcd_st7735.h"
#include "spi.h"

// Constants.
enum {
  // Pin out mapping.
  LcdCsPin = 0,
  LcdRstPin,
  LcdDcPin,
  LcdBlPin,
  LcdMosiPin,
  LcdSclkPin,
  // Spi clock rate.
  LcdSpiSpeedHz = 5 * 100 * 1000,
};

// Buttons
// The direction is relative to the screen in landscape orientation.
typedef enum {
  BTN_DOWN  = 0b00001,
  BTN_LEFT  = 0b00010,
  BTN_CLICK = 0b00100,
  BTN_RIGHT = 0b01000,
  BTN_UP    = 0b10000,
} Buttons_t;

enum {
  BGRColorBlack = 0x000000,
  BGRColorBlue  = 0xFF0000,
  BGRColorGreen = 0x00FF00,
  BGRColorRed   = 0x0000FF,
  BGRColorWhite = 0xFFFFFF,
};

int lcd_init(spi_t *spi, St7735Context *lcd, LCD_Interface *interface);

#endif // LCD_H
