#ifndef SENSORS_TFT_H
#define SENSORS_TFT_H

// A. Sheaff 2023/01/05

enum adc_channel {
	adc_left=0,		// Left
	adc_front=1,	// Front
	adc_right=2		// Right
};

// Wait for a key press
uint8_t keypress(void);
// Get a sensor channel - uses averaging
uint16_t get_sensor(enum adc_channel adcc, uint32_t samples);
// Get and ADC value (read sensor)
uint16_t get_adc(enum adc_channel adcc);
// Core 1 thread to display data and read sensors
void sensors_display(void);

extern volatile int32_t left,front,right;		// Sensor average
extern volatile struct motion *dmotors;	// Pointer to display target period on TFT
extern volatile uint32_t supdate;

#endif // SENSORS_TFT_H
