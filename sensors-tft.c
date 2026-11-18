// A. Sheaff 2023/01/05
#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "class/cdc/cdc_device.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "pico/binary_info.h"
#include "Adafruit_4383.h"
#include "graphics.h"
#include "bootrom_api.h"
#include "tft_stdout.h"
#include "sensors-tft.h"
#include "motors.h"
#include "cmsis_gcc.h"

volatile int32_t left=0,front=0,right=0;		// Sensor average
volatile uint32_t supdate=0;
volatile struct motion *dmotors;	// Pointer to display target period on TFT

uint16_t get_sensor(enum adc_channel adcc, uint32_t samples)
{
	uint32_t s=0;	// Sum and average at end

	adc_select_input(adcc);

	// Take sample samples
	for (uint32_t i=0;i<samples;i++) {
		s+=adc_read();
	}

	// Return average
	return (s/samples);
}

void update_sensor_values() {
	front=get_sensor(adc_front,256);
	left=get_sensor(adc_left,256);
	right=get_sensor(adc_right,256);
}

// Get a sensor reading
uint16_t get_adc(enum adc_channel adcc)
{
	// Get the sensor value
	return adc_read();
}

// Waits for a keypress over USB
uint8_t keypress(void)
{
	while(!tud_cdc_available());
	return getchar();
}

// Displays data on TFT and reads/updates/averages sensor values
// Handles USB keypresses
void sensors_display(void)
{
	signed char d;		// Keypress
	uint32_t j=0;		// Counter
	uint8_t buf[16];

	// Sensor Reset / Channel change GP21
	gpio_init(21);
	gpio_put(21,0);
	gpio_set_dir(21,GPIO_OUT);

	// ADC GP28/ADC0
	adc_init();	
	adc_gpio_init(26);
	adc_select_input(0);

	// TFT is at 53 (135) 40 (239) for corner on rotated display (widescreen)
	// TFT Init Reset GP15 SPI GP16-GP19 DC GP20
	Adafruit_4383_init(240,135);
	Adafruit_4383_setRotation(0);
	Adafruit_4383_fillScreen(0x0000);
	Adafruit_4383_drawFastVLine(0,0,135,0xFFFF);
	Adafruit_4383_drawFastHLine(0,0,240,0xFFFF);
	Adafruit_4383_drawFastVLine(239,0,135,0xFFFF);
	Adafruit_4383_drawFastHLine(0,134,240,0xFFFF);
	sleep_ms(1000);

	// Get the graphics initialized
	graphics_init(240,135);
	graphics_setTextWrap(false);
	graphics_setTextColor(0xFFFF);
	graphics_setTextSize(3);

	while (1) {
		// Reboot to program mode with 'f'
		if (tud_cdc_available()) {
			d=getchar();	// Get the first received character
			switch (d) {
				case 'f':	// if the key is 'f' reboot to Mass Storage Mode
					reset_usb_boot(0,0);
					break;
			}
		}

		// Get sensor values
		update_sensor_values();

		// Display sensor values
		if (j%8==0) {
			supdate=1;

			// Draw the text on the TFT
			graphics_fillRect(1,1,238,10,0x0000);
			// Get sensor values
			update_sensor_values();
			graphics_fillRect(5,30,150,25,0x0000);
			snprintf((char *)buf,32,"%4u",left);
			graphics_drawText((char *)buf,5,30);
			// Get sensor values
			update_sensor_values();
			graphics_fillRect(95,5,120,25,0x0000);
			snprintf((char *)buf,32,"%4u",front);
			graphics_drawText((char *)buf,95,5);
			// Get sensor values
			update_sensor_values();
			graphics_fillRect(150,30,80,25,0x0000);
			snprintf((char *)buf,32,"%4u",right);
			graphics_drawText((char *)buf,150,30);
		}
		j++;
	}
}
