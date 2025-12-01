#pragma once
#include "common.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

// 4988 stepper controller chip implementation
typedef struct StepperIO {
	GPIO_TypeDef* gpio;
	// We assume these are all on the same bus
	uint16_t step;
	uint16_t dir;
	uint16_t reset;
	uint16_t enable;
	uint16_t ms1;
	uint16_t ms2;
	uint16_t ms3;
} StepperIO;

typedef enum StepperMode {
	STEPM_FULL = 0b000,
	STEPM_HALF = 0b001,
	STEPM_QUARTER = 0b010,
	STEPM_EIGHTH = 0b011,
	STEPM_SIXTEENTH = 0b111
} StepperMode;

typedef enum StepperDirection {
	STEPD_CLOCKWISE, STEPD_COUNTERCLOCKWISE
} StepperDirection;

typedef struct Stepper {
	StepperIO io;
	StepperMode mode;
	StepperDirection dir;
	bool is_simplified;
} Stepper;

// Creates a new Stepper with mode defaulted to SM_FULL
Stepper Stepper_new(GPIO_TypeDef* bus);

void Stepper_set_mode(Stepper* self, StepperMode mode);
void Stepper_set_direction(Stepper* self, StepperDirection sd);

void Stepper_init(Stepper* self);
void Stepper_init_simplified(Stepper *self);

void Stepper_step(Stepper* self);