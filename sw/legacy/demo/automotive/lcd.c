// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "lcd.h"

#include "core/lucida_console_10pt.h"
#include "core/m3x6_16pt.h"
#include "gpio.h"
#include "sonata_system.h"
#include "spi.h"
#include "st7735/lcd_st7735.h"
#include "timer.h"

static void timer_delay(uint32_t ms) {
  uint32_t timeout = get_elapsed_time() + ms;
  while (get_elapsed_time() < timeout) {
    asm volatile("wfi");
  }
}

static uint32_t spi_write(void *handle, uint8_t *data, size_t len) {
  spi_tx(handle, data, len);
  spi_wait_idle(handle);
  return len;
}

static uint32_t gpio_write(void *handle, bool cs, bool dc) {
  set_output_bit(GPIO_OUT, LcdDcPin, dc);
  set_output_bit(GPIO_OUT, LcdCsPin, cs);
  return 0;
}

int lcd_init(spi_t *spi, St7735Context *lcd, LCD_Interface *interface) {
  // Set the initial state of the LCD control pins
  set_output_bit(GPIO_OUT, LcdDcPin, 0x0);
  set_output_bit(GPIO_OUT, LcdBlPin, 0x1);
  set_output_bit(GPIO_OUT, LcdCsPin, 0x0);

  // Reset the LCD
  set_output_bit(GPIO_OUT, LcdRstPin, 0x0);
  timer_delay(150);
  set_output_bit(GPIO_OUT, LcdRstPin, 0x1);

  // Init LCD Driver, and set the SPI driver
  interface->handle      = spi;          // SPI handle.
  interface->spi_write   = spi_write;    // SPI write callback.
  interface->gpio_write  = gpio_write;   // GPIO write callback.
  interface->timer_delay = timer_delay;  // Timer delay callback.
  lcd_st7735_init(lcd, interface);

  // Set the LCD orientation
  lcd_st7735_set_orientation(lcd, LCD_Rotate180);

  // Setup text font bitmaps to be used and the colors.
  lcd_st7735_set_font(lcd, &m3x6_16ptFont);
  lcd_st7735_set_font_colors(lcd, BGRColorWhite, BGRColorBlack);

  // Clean display with a white rectangle.
  lcd_st7735_clean(lcd);
  
  return 0;
}