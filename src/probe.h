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
} Probe;

typedef struct Bed {
	Stepper stepper;
	GPIO_TypeDef* gpio;
	uint16_t homing;
} Bed;

typedef struct ProbeSet {
	Probe left;
	Probe right;
	Bed bed;
} ProbeSet;

extern ProbeSet probes;

typedef struct ProbePosition {
	float rotation;
	int32_t position;
} ProbePosition;

void ProbeSet_init(ProbeSet* probes);

ProbePosition Probe_calculate_position(Side side, uint32_t x, uint32_t y);
void Probe_to_location(Probe* probe, ProbePosition position);

bool ProbeSet_test_continuity(ProbeSet* probes);

void ProbeSet_lower_bed(ProbeSet* probes);
void ProbeSet_raise_bed(ProbeSet* probes);

