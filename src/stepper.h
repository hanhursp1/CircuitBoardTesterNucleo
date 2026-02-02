#pragma once
#include "common.h"
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdint.h>
#include "config.h"

#pragma region Type Declarations

// 4988 stepper controller chip implementation
typedef struct StepperIO {
	// The GPIO bus of the stepper. We assume these are all on the same bus
	GPIO_TypeDef* gpio;
	// Pin to toggle for stepping
	uint16_t step;
	// Pin representing direction
	uint16_t dir;
	// Reset pin
	uint16_t reset;
	// Enable pin
	uint16_t enable;
	// Mode Set pins 1 through 3
	uint16_t ms1;
	uint16_t ms2;
	uint16_t ms3;
} StepperIO;

typedef enum StepperMode {
	// Full step mode
	STEPM_FULL = 0b000,
	// Half step mode
	STEPM_HALF = 0b001,
	// Quarter step mode
	STEPM_QUARTER = 0b010,
	// Eighth step mode
	STEPM_EIGHTH = 0b011,
	// Sixteenth step mode
	STEPM_SIXTEENTH = 0b111
} StepperMode;

typedef enum StepperDirection {
	STEPD_CLOCKWISE, STEPD_COUNTERCLOCKWISE
} StepperDirection;

#define STEPD_FORWARDS STEPD_CLOCKWISE
#define STEPD_BACKWARDS STEPD_COUNTERCLOCKWISE

typedef struct Stepper {
	// I/O for the stepper
	StepperIO io;
	// Mode of the stepper
	StepperMode mode;
	// Direction of the stepper
	StepperDirection dir;
	// Position of the stepper along its axis, in micrometers
	int32_t position;
	// Whether or not the stepper is a simplified version
	// (Only io.step and io.dir are set, mode is always STEPM_FULL)
	bool is_simplified;
} Stepper;

typedef struct StepperQueueEntry {
	Stepper* stepper;
	uint32_t num_steps;
	StepperDirection dir;
	bool is_high;
} StepperQueueEntry;

#pragma endregion

#pragma region Global Variable Declarations

extern int um_per_step;
extern StepperQueueEntry stepper_queue[MAX_STEPPERS];

#pragma endregion

#pragma region Function Declarations

void Stepper_set_mode(Stepper* self, StepperMode mode);
void Stepper_set_direction(Stepper* self, StepperDirection sd);

void Stepper_init(Stepper* self);
void Stepper_init_simplified(Stepper *self);

void Stepper_step_immediate(Stepper* self);
void Stepper_move_to_immediate(Stepper *self, uint32_t position);


// TODO: Implement a stepper queue system so that multiple steppers can move at once

// Queue the stepper a set amount of steps in the given direction. Returns true if
// it was able to queue successfully
bool Stepper_queue_steps(Stepper* self, uint32_t step_count, StepperDirection dir);
// Move the stepper to the given position from home, in um
void Stepper_move_to(Stepper* self, uint32_t position);

// TODO: Implement the queue using a timer interrupt (maybe hook it into the systick?)
void StepperQueueEntry_operate(StepperQueueEntry* entry);

void StepperQueue_iterate();

bool StepperQueue_has(Stepper* s);

#pragma endregion