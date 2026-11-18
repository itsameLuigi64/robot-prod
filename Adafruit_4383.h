#ifndef ADAFRUIT_4383_H
#define ADAFRUIT_4383_H

#include <stdint.h>


/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
    ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
    ----> https://www.adafruit.com/product/2088
  as well as Adafruit raw 1.8" TFT display
    ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <stdint.h>
#include "hardware/gpio.h"

#define SPI_MODE0 0

#define SPI_DEFAULT_FREQ 32000000 ///< Default SPI data clock frequency

extern uint8_t _colstart;   ///< Some displays need this changed to offset
extern uint8_t _rowstart;       ///< Some displays need this changed to offset
extern uint8_t spiMode; ///< Certain display needs MODE3 instead
extern uint8_t rotation;
extern uint16_t _xstart, _ystart, _width, _height;
extern uint32_t _freq;

#define LCD_TX_PIN 19    // TX MOSI
#define LCD_RX_PIN 16    // RX MISO
#define LCD_CK_PIN 18   // Clock
#define LCD_DC_PIN 20   // Data/Command
#define LCD_BL_PIN 11   // Backlight
#define LCD_RT_PIN 15   // Reset
#define LCD_CS_PIN 17   // Chip Select

#define ST7735_TFTWIDTH_128 128  // for 1.44 and mini
#define ST7735_TFTWIDTH_80 80    // for mini
#define ST7735_TFTHEIGHT_128 128 // for 1.44" display
#define ST7735_TFTHEIGHT_160 160 // for 1.8" and mini display

#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

// Some ready-made 16-bit ('565') color settings:
#define ST77XX_BLACK 0x0000
#define ST77XX_WHITE 0xFFFF
#define ST77XX_RED 0xF800
#define ST77XX_GREEN 0x07E0
#define ST77XX_BLUE 0x001F
#define ST77XX_CYAN 0x07FF
#define ST77XX_MAGENTA 0xF81F
#define ST77XX_YELLOW 0xFFE0
#define ST77XX_ORANGE 0xFC00

void Adafruit_4383_CS_HIGH(void);
void Adafruit_4383_CS_LOW(void);
void Adafruit_4383_DC_HIGH(void);
void Adafruit_4383_DC_LOW(void);

void Adafruit_4383_init(uint16_t width, uint16_t height);
void Adafruit_4383_displayInit(const uint8_t *addr);
void Adafruit_4383_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void Adafruit_4383_enableDisplay(uint32_t enable);
void Adafruit_4383_enableTearing(uint32_t enable);
void Adafruit_4383_enableSleep(uint32_t enable);


void Adafruit_4383_begin(uint32_t freq);
void Adafruit_4383_commonInit(const uint8_t *cmdList);
void Adafruit_4383_displayInit(const uint8_t *addr);
void Adafruit_4383_setColRowStart(int8_t col, int8_t row);

void Adafruit_4383_setRotation(uint8_t m);
void Adafruit_4383_fillScreen(uint16_t color);
void Adafruit_4383_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void Adafruit_4383_drawPixel(int16_t x, int16_t y, uint16_t color);

void Adafruit_4383_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void Adafruit_4383_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
#endif // ADAFRUIT_4383_H
