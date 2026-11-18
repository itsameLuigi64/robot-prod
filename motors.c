// A. Sheaff 2023/01/05
#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"
#include "motors.h"

volatile uint32_t lupdate=0;	// Motor step left update falg
volatile uint32_t rupdate=0;	// Motor step right update falg

// Stop the robot - power down the motors
void stop(struct motion *motors)
{
	pwm_set_enabled(motors->slice9, false);	// Turn off PWM
	pwm_set_enabled(motors->slice10, false);
	gpio_put_masked(0x000000FF,0);	// Motors off

	while (1);	// All stop - Core 1 keeps going with sensor values
}

// Limit v between the lower limit ll and upper limit ul
int32_t limit(int32_t v, int32_t ll, int32_t ul)
{
	if (v<ll) {
		return ll;
	}
	if (v>ul) {
		return ul;
	}
	return v;
}

// Update the motors period - does NOT make the motor spin
int32_t mupdate(int32_t tper, int32_t per, uint slice)
{
	// If needs to update
	if (tper!=per) {
		if (tper<per) {
			per=per*993/1000;	// Acceleration
		} else {
			per=tper;
		}
		// Limit speed
		per=limit(per,MAXSPEED,MINSPEED);
		// Set new period
		pwm_set_wrap(slice, per);
	}
	return per;
}

// PWM timer (one for each motor) sets the step update period
// After the timer finishes one period, a step is taken
void pwm_isr(void)
{
	// Left step sequence
	static const uint8_t lsteps[]={0x08,0x04,0x02,0x01};
	// Right step sequence
	static const uint8_t rsteps[]={0x10,0x20,0x40,0x80};
	// Left step index
	static uint8_t lmb=0;
	// Right step index
	static uint8_t rmb=0;

	// Turn on External LED to indicate we are moving
	gpio_put(8,1);

	// Get who generated the interrupt
	uint32_t im=pwm_get_irq_status_mask();
	// Clear the interrupt(s)
	pwm_hw->intr=im;
	// If slice 10 then step right motor steps
	if (im&(1<<5)) {
		rmb=(rmb+1)&0x03;
		rupdate=1;
	}
	// If slice 9 then left motor steps
	if (im&(1<<4)) {
		lmb=(lmb+1)&0x03;
		lupdate=1;
	}
	// Write out the new step values (turns on/off one coil on each stepper motor)
	gpio_put_masked(0x000000FF,rsteps[rmb]|lsteps[lmb]);
	// Blink the onboard LED
	gpio_xor_mask(1<<25);
}

