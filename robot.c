// A. Sheaff 2023/01/05
#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "sensors-tft.h"
#include "motors.h"

//Uses 2 Vexta PX245-02B-C8 Stepper Motors With a 1.8 deg angle


// Entry point
int main(void)
{
	uint32_t i;		// Counter
	int32_t err;	// Left - Right error
	int32_t f_err;	// Front left - Front Right error

	struct motion motors={	//	Initial periods for all motors
		.tlper=10000,
		.trper=10000,
		.lper=10000,
		.rper=10000
	};

	// ****  INITIALIZE the Robot ****

	// Setup so we can print motor values on the TFT
	dmotors=&motors;

	// Setup stdio - stdio_init_all() directs to USB CDC
	// stdio_set_driver_enabled() sets up the TFT for stdout
	// No stdin is set up with the TFT
	stdio_init_all();
	//stdio_set_driver_enabled(&stdio_tft, true);

	// Allow the USB to enumerate
	sleep_ms(1000);

	// Regulator in PWM mode
	gpio_init(23);
	gpio_put(23,1);
	gpio_set_dir(23,GPIO_OUT);

	// Onboard LED GP25
	gpio_init(25);
	gpio_put(25,0);
	gpio_set_dir(25,GPIO_OUT);

	// PWM IR LEDs GP22
	// Not used

	// External LED GP08
	gpio_init(8);
	gpio_put(8,0);
	gpio_set_dir(8,GPIO_OUT);

	// MUX GP12-GP14
	// 000 is right front
	// 001 is left  front
	// 010 is left
	// 011 is right
	gpio_init(12);
	gpio_init(13);
	gpio_init(14);
	gpio_put(12,0);
	gpio_put(13,0);
	gpio_put(14,0);
	gpio_set_dir(12,GPIO_OUT);
	gpio_set_dir(13,GPIO_OUT);
	gpio_set_dir(14,GPIO_OUT);

	// Launch core1 to display data and update sensors
	multicore_launch_core1(sensors_display);

	// Motors Left GP00-GP03 Right GP04-GP07
	for (i=0;i<8;i++) {
		gpio_init(i);
		gpio_put(i,0);
		gpio_set_dir(i,GPIO_OUT);
	}

	// Use GP09 & GP10 for step timing
	gpio_set_function(9,GPIO_FUNC_PWM);	// Channel 4B
	gpio_set_function(10,GPIO_FUNC_PWM);	// Channel 5A
	motors.slice9 = pwm_gpio_to_slice_num(9);	// Left Motor
	motors.slice10 = pwm_gpio_to_slice_num(10);	// Right Motor
	pwm_clear_irq(motors.slice9);	// Clear old interrupts
	pwm_clear_irq(motors.slice10);
	irq_set_exclusive_handler(PWM_IRQ_WRAP,pwm_isr);	// Set handler
	irq_set_enabled(PWM_IRQ_WRAP,true);	// Enable interrupt - global
	pwm_set_irq_mask_enabled((1<<motors.slice9)|(1<<motors.slice10),true);	// Enable interrup - local
	pwm_set_wrap(motors.slice9, motors.lper);	// Set initial period
	pwm_set_wrap(motors.slice10, motors.rper);
	pwm_set_chan_level(motors.slice9, PWM_CHAN_B, 250);	// High time doesn't really matter
	pwm_set_chan_level(motors.slice10, PWM_CHAN_A, 250);
	pwm_set_clkdiv_int_frac4(motors.slice9,255,0);	// Prescale by 255
	pwm_set_clkdiv_int_frac4(motors.slice10,255,0);


	// Wait for sensor data to be ready
	sleep_ms(1000);

	// Wait for both front sensors to be close
	// Waive hand in front of front sensors to start robot
	while (front<400) {
	}

	// Delay before takeoff
	sleep_ms(1000);

	// Turn on PWM
	pwm_set_enabled(motors.slice9, true);
	pwm_set_enabled(motors.slice10, true);
	
	// Stepper motor is 200 steps/rev
	// Wheel Diameter is 2.111 inches
	// Wheel base is 5.3"
	// Pico is clocked at 125 MHz
	// PWM are prescaled at 255 (divide)
	//   And then by the PWM count
	// At higher speed the motors/wheels appear to slip causing large turn radii

	// Prepare to move out of the gate quickly
	motors.tlper=3500;
	motors.trper=3500;

	// *** ALL LOGIC AND CONTROL IN THIS while LOOP ****
	while (true) {
		err=left-right;	// Left/right error
		f_err=front;	// Front error

		if (supdate) {
			supdate=0;
			if (err>200) {	// Left is too close - go right
				if (f_err>200) {	// If the front sees a wall
					motors.tlper=3500;
					motors.trper=3500+5*err+f_err*f_err;
				} else if (f_err<-200) {
				} else {	// No wall out front
					motors.tlper=3500;
					motors.trper=3500+5*err;
				}
			} else if (err<-200) {	// Right too close - go left
				if (f_err>200) {
				} else if (f_err<-200) {	// If the front sees a wall
					motors.tlper=3500-5*err+f_err*f_err;
					motors.trper=3500;
				} else {	// No wall out front
					motors.tlper=3500-5*err;
					motors.trper=3500;
				}
			} else {
				if (motors.tlper<motors.trper) {
					motors.trper=motors.tlper;
				} else {
					motors.tlper=motors.trper;
				}
			}
		}
		// Stop if either front sensor is too close
		if (front>2000) {
			stop(&motors);
		}

		// Update left motor - DO NOT MODIFY
		if (lupdate) {
			lupdate=0;
			motors.lper=mupdate(motors.tlper,motors.lper,motors.slice9);
		}
		// Update right motor - DO NOT MODIFY
		if (rupdate) {
			rupdate=0;
			motors.rper=mupdate(motors.trper,motors.rper,motors.slice10);
		}
	}
}
