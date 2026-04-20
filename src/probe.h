#pragma once

#include "servo.h"
#include "stepper.h"
#include "stm32f446xx.h"
#include <stdint.h>

typedef struct ProbeIO {
	GPIO_TypeDef* gpio;
	uint16_t probe_pin;
	uint16_t homing_pin;
} ProbeIO;

typedef struct Probe {
	// The stepper motor that serves as the rail
	Stepper rail;
	// The servo motor that operates the probe itself
	Servo axis;
	// The probe's own IO
	ProbeIO io;
	// Whether or not the probe pin is an output
	bool pin_is_output;
} Probe;

typedef struct Bed {
	Servo servo;
	GPIO_TypeDef* gpio;
	uint16_t homing;
} Bed;

typedef struct ProbeSet {
	Probe left;
	Probe right;
	Bed bed;
} ProbeSet;

extern ProbeSet probes;