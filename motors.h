#ifndef MOTORS_H
#define MOTORS_H

// A. Sheaff 2023/01/05
#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define MAXSPEED (3000)
#define MINSPEED (10000)

// Track motion data
struct motion {
	int32_t tlper;	// Target period left
	int32_t trper;	// Target period right
	volatile int32_t lper;	// Actual period left
	volatile int32_t rper;	// Actual period right
	uint slice9;	// PWM Left
	uint slice10;	// PWM Right
};

// Limit value v
int32_t limit(int32_t v, int32_t ll, int32_t ul);
// Update the steps
int32_t mupdate(int32_t tper, int32_t per, uint slice);
// Stop the robot and turn off motors
void stop(struct motion *motors);
// ISR to flag motor update
void pwm_isr(void);

extern volatile uint32_t lupdate;	// Motor step left update falg
extern volatile uint32_t rupdate;	// Motor step right update falg

extern volatile struct motion *dmotors;	// Pointer to display target period on TFT

#endif // MOTORS_H
