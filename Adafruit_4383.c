//  Written by Limor Fried/Ladyada for Adafruit Industries.
//  MIT license, all text above must be included in any redistribution

//  Modified by A Sheaff

//  For use with Adafruit 4383 & ST7789

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <cmsis_gcc.h>
#include "Adafruit_4383.h"
#include "hardware/spi.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "class/cdc/cdc_device.h"

uint16_t windowWidth;
uint16_t windowHeight;

uint8_t _colstart = 0;		///< Some displays need this changed to offset
uint8_t _rowstart = 0;		///< Some displays need this changed to offset
uint8_t rotation = 0;
uint16_t _xstart, _ystart, _width, _height;

// ST7789 240RGB(H) x 320 (V)
//        Col         Lines
//        X           Y
//        0-239       0-319
// Display 135RGB(H) x 240 (V)
// CASET is X
// RASET is Y
static const uint8_t tft_init_4383[] = {
	9,
	ST_CMD_DELAY | 0, ST77XX_SWRESET, 150,
	ST_CMD_DELAY | 0, ST77XX_SLPOUT, 255,
	ST_CMD_DELAY | 1, ST77XX_COLMOD, 0x55, 10,
	1, ST77XX_MADCTL, 0x68, // 0x08,
	4, ST77XX_CASET, 0x00, 53, 0x00, 134+53, // XS MBS, XS LSB, XE MSB, XE LSB
	4, ST77XX_RASET, 0x00, 40, 0x01, (239+40)&0xFF, //	YS MBS, YS LSB, YE MSB, YE LSB
	0, ST77XX_INVON,
	ST_CMD_DELAY | 0, ST77XX_NORON, 10,
	ST_CMD_DELAY | 0, ST77XX_DISPON, 255
};

void Adafruit_4383_CS_HIGH(void)
{
	gpio_put(LCD_CS_PIN, 1);
}

void Adafruit_4383_CS_LOW(void)
{
	gpio_put(LCD_CS_PIN, 0);
}

void Adafruit_4383_DC_HIGH(void)
{
	gpio_put(LCD_DC_PIN, 1);
}

void Adafruit_4383_DC_LOW(void)
{
	gpio_put(LCD_DC_PIN, 0);
}

inline uint16_t swapcolor(uint16_t x)
{
	return (x << 11) | (x & 0x07E0) | (x >> 11);
}

void Adafruit_4383_writecommand(uint8_t c)
{
	Adafruit_4383_DC_LOW();
	Adafruit_4383_CS_LOW();

	spi_write_blocking(spi0, &c, 1);

	Adafruit_4383_CS_HIGH();
}

void Adafruit_4383_writedata(uint8_t c)
{
	Adafruit_4383_DC_HIGH();
	Adafruit_4383_CS_LOW();

	spi_write_blocking(spi0, &c, 1);

	Adafruit_4383_CS_HIGH();
}

void Adafruit_4383_writecommandblock(const uint8_t *d, uint16_t cnt)
{

	Adafruit_4383_DC_LOW();
	Adafruit_4383_CS_LOW();

	spi_write_blocking(spi0, &d[0], 1);
	if (cnt>1) {
		Adafruit_4383_DC_HIGH();
		spi_write_blocking(spi0, &d[1], cnt-1);
	}

	Adafruit_4383_CS_HIGH();
}


void Adafruit_4383_writedatablock(uint8_t *d, uint16_t cnt)
{
	Adafruit_4383_DC_HIGH();
	Adafruit_4383_CS_LOW();

	spi_write_blocking(spi0, d, cnt);

	Adafruit_4383_CS_HIGH();
}

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST7789 displays
    @param  width  Display width
    @param  height Display height
    @param  mode   SPI data mode; one of SPI_MODE0, SPI_MODE1, SPI_MODE2

                   or SPI_MODE3 (do NOT pass the numbers 0,1,2 or 3 -- use
                   the defines only, the values are NOT the same!)
*/
/**************************************************************************/
void Adafruit_4383_init(uint16_t width, uint16_t height)
{
	spi_init(spi0, 16 * 1000 * 1000);
	gpio_set_function(LCD_CK_PIN, GPIO_FUNC_SPI);	// Clock
	gpio_set_function(LCD_TX_PIN, GPIO_FUNC_SPI);	// TX MOSI
	gpio_set_function(LCD_RX_PIN, GPIO_FUNC_SPI);	// RX MISO

	gpio_init(LCD_CS_PIN);	// CS
	gpio_set_dir(LCD_CS_PIN, GPIO_OUT);
	gpio_put(LCD_CS_PIN, 1);

	gpio_init(LCD_DC_PIN);	// D/C
	gpio_set_dir(LCD_DC_PIN, GPIO_OUT);
	gpio_put(LCD_DC_PIN, 0);

	gpio_init(LCD_BL_PIN);	// BL
	gpio_set_dir(LCD_BL_PIN, GPIO_OUT);
	gpio_put(LCD_BL_PIN, 1);

	gpio_init(LCD_RT_PIN);	// Reset
	gpio_set_dir(LCD_RT_PIN, GPIO_OUT);
	gpio_put(LCD_RT_PIN, 1);

	Adafruit_4383_CS_HIGH();
	sleep_ms(20);
	gpio_put(LCD_RT_PIN, 0);
	sleep_ms(50);
	gpio_put(LCD_RT_PIN, 1);
	sleep_ms(150);

	// 1.14" display (centered, with odd size)
	_rowstart = 53;
	_colstart = 40;
	_width=width;
	_height=height;
	_xstart=53;
	_ystart=40;

	windowWidth = width;
	windowHeight = height;

	Adafruit_4383_displayInit(tft_init_4383);

	Adafruit_4383_writecommand(ST77XX_RAMWR);
}

/**************************************************************************/
/*!
    @brief  Companion code to the initiliazation tables. Reads and issues
            a series of LCD commands stored in PROGMEM byte array.
    @param  addr  Flash memory array with commands and data to send
*/
/**************************************************************************/
void Adafruit_4383_displayInit(const uint8_t *addr)
{

	uint8_t numCommands, numArgs;
	uint16_t ms;

	numCommands = *(addr++);	// Number of commands to follow
	while (numCommands--) {	// For each command...
		numArgs = *(addr++);	// Number of args to follow
		ms = numArgs & ST_CMD_DELAY;	// If hibit set, delay follows args
		numArgs &= ~ST_CMD_DELAY;	// Mask out delay bit
		Adafruit_4383_writecommandblock(addr, numArgs+1);
		addr += numArgs+1;

		if (ms) {
			ms = *(addr++);	// Read post-command delay time (ms)
			if (ms == 255)
				ms = 500;	// If 255, delay for 500 ms
			sleep_ms(ms);
		}
	}
}

/**************************************************************************/
/*!
  @brief  SPI displays set an address window rectangle for blitting pixels
  @param  x  Top left corner x coordinate
  @param  y  Top left corner y coordinate
  @param  w  Width of window
  @param  h  Height of window
*/
/**************************************************************************/
void Adafruit_4383_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	uint16_t s=x0+_xstart;
	uint16_t e=x1+_xstart;
    Adafruit_4383_writecommand(ST77XX_CASET); // Column addr set              
    Adafruit_4383_writedata(s>>8);                                            
    Adafruit_4383_writedata(s&0xFF); // XSTART                           
    Adafruit_4383_writedata(e>>8);                                            
    Adafruit_4383_writedata(e&0xFF); // XEND                             
                                                                                
	s=y0+_ystart;
	e=y1+_ystart;
    Adafruit_4383_writecommand(ST77XX_RASET); // Row addr set                 
    Adafruit_4383_writedata(s>>8);                                            
    Adafruit_4383_writedata(s&0xFF); // YSTART                           
    Adafruit_4383_writedata(e>>8);                                            
    Adafruit_4383_writedata(e&0xFF); // YEND                             
}

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Adafruit_4383_setRotation(uint8_t m)
{
	//uint8_t madctl = 0;

	rotation = m % 4;	// can't be higher than 3

	switch (rotation) {
	case 0:
	//	madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
		_xstart = _colstart;
		_ystart = _rowstart;
		break;
	case 1:
	//	madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
		_ystart = _colstart;
		_xstart = _rowstart;
		break;
	case 2:
	//	madctl = ST77XX_MADCTL_RGB;
		_xstart = _colstart;
		_ystart = _rowstart;
		break;
	case 3:
	//	madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
		_ystart = _colstart;
		_xstart = _rowstart;
		break;
	}

//  sendCommand(ST77XX_MADCTL, &madctl, 1);
//	Adafruit_4383_writecommand(madctl);
}

void Adafruit_4383_fillScreen(uint16_t color)
{
	Adafruit_4383_fillRect(0, 0, _width, _height, color);
}

// fill a rectangle                                                             
void Adafruit_4383_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	// rudimentary clipping (drawChar w/big text requires this)                 
	if ((x >= _width) || (y >= _height))
		return;
	if ((x + w - 1) >= _width)
		w = _width - x;
	if ((y + h - 1) >= _height)
		h = _height - y;

	//printf("*%u %u %u %u\r\n", x, y, x + w - 1, y + h - 1);
	Adafruit_4383_setAddrWindow(x, y, x + w - 1, y + h - 1);

                                                                                
	uint8_t cmd=ST77XX_RAMWR;
	uint16_t line[w];
	for (uint32_t i = 0; i < w; i++) {
		line[i] = color;
	}
	Adafruit_4383_DC_LOW();
	Adafruit_4383_CS_LOW();
	spi_write_blocking(spi0, &cmd, 1);
	Adafruit_4383_DC_HIGH();
	for (y = h; y > 0; y--) {
		spi_write_blocking(spi0, (uint8_t *) line, w * sizeof(uint16_t));
	}
	Adafruit_4383_CS_HIGH();
}
/**************************************************************************/
/*!
    @brief  Set origin of (0,0) of display with offsets
    @param  col  The offset from 0 for the column address
    @param  row  The offset from 0 for the row address
*/
/**************************************************************************/
void Adafruit_4383_setColRowStart(int8_t col, int8_t row)
{
	_colstart = col;
	_rowstart = row;
}

/**************************************************************************/
/*!
 @brief  Change whether display is on or off
 @param  enable True if you want the display ON, false OFF
 */
/**************************************************************************/
void Adafruit_4383_enableDisplay(uint32_t enable)
{
//  sendCommand(enable ? ST77XX_DISPON : ST77XX_DISPOFF);
	uint8_t cmd = enable ? ST77XX_DISPON : ST77XX_DISPOFF;
	Adafruit_4383_writecommand(cmd);
}

void Adafruit_4383_drawPixel(int16_t x, int16_t y, uint16_t color)
{

	if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
		return;

	Adafruit_4383_setAddrWindow(x, y, x + 1, y + 1);

	uint8_t cmd=ST77XX_RAMWR;
	Adafruit_4383_DC_LOW();
	Adafruit_4383_CS_LOW();
	spi_write_blocking(spi0, &cmd, 1);
	Adafruit_4383_DC_HIGH();
	spi_write_blocking(spi0,(uint8_t *)&color,2);
	Adafruit_4383_CS_HIGH();

}

void Adafruit_4383_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{

	// Rudimentary clipping
	if ((x >= _width) || (y >= _height))
		return;
	if ((y + h - 1) >= _height)
		h = _height - y;
	Adafruit_4383_setAddrWindow(x, y, x, y + h - 1);

	uint16_t line[h];
	for (uint32_t i=0;i<h;i++) {
		line[i]=color;
	}

	uint8_t cmd=ST77XX_RAMWR;
	Adafruit_4383_DC_LOW();
	Adafruit_4383_CS_LOW();
	spi_write_blocking(spi0, &cmd, 1);
	Adafruit_4383_DC_HIGH();
	spi_write_blocking(spi0,(uint8_t *)line,h*sizeof(uint16_t));
	Adafruit_4383_CS_HIGH();
}


void Adafruit_4383_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{

	// Rudimentary clipping
	if ((x >= _width) || (y >= _height))
		return;
	if ((x + w - 1) >= _width)
		w = _width - x;
	Adafruit_4383_setAddrWindow(x, y, x + w - 1, y);

	uint16_t line[w];
	for (uint32_t i=0;i<w;i++) {
		line[i]=color;
	}

	uint8_t cmd=ST77XX_RAMWR;
	Adafruit_4383_DC_LOW();
	Adafruit_4383_CS_LOW();
	spi_write_blocking(spi0, &cmd, 1);
	Adafruit_4383_DC_HIGH();
	spi_write_blocking(spi0,(uint8_t *)line,w*sizeof(uint16_t));
	Adafruit_4383_CS_HIGH();
}
