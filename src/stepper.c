#include "stepper.h"
#include "common.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "config.h"

#pragma region Global Variable Definitions
// This is non-constant to allow it to be updated from a config function.
int um_per_step = 100;

// Stepper movement queue to be iterated over each interrupt
StepperQueueEntry stepper_queue[MAX_STEPPERS] = {0};

#pragma endregion

#pragma region Function Definitions

bool valid_stepper_io(StepperIO* io) {
	return (
		io->step && io->dir && io->reset && io->enable && io->ms1 && io->ms2 && io->ms3
	);
}

bool valid_stepper_simplified_io(StepperIO* io) {
	return (
		io->step && io->dir
	);
}

#pragma region Stepper

void Stepper_step_immediate(Stepper* self) {
	HAL_GPIO_TogglePin(self->io.gpio, self->io.step);

	// TODO: Make this delay shorter somehow. Maybe add a custom timer implementation?
	HAL_Delay(1);

	HAL_GPIO_TogglePin(self->io.gpio, self->io.step);

	// Update the stepper's position
	self->position += (self->dir == STEPD_FORWARDS) ? um_per_step : -um_per_step;
}

void Stepper_set_mode(Stepper* self, StepperMode mode) {
	assert(!self->is_simplified);
	self->mode = mode;
	// TODO: update pins

	HAL_GPIO_WritePin(self->io.gpio, self->io.ms1, (mode & 0b001) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(self->io.gpio, self->io.ms2, (mode & 0b010) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(self->io.gpio, self->io.ms3, (mode & 0b100) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Stepper_set_direction(Stepper* self, StepperDirection sd) {
	self->dir = sd;
	HAL_GPIO_WritePin(self->io.gpio, self->io.dir, (sd == STEPD_CLOCKWISE) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Stepper_init(Stepper *self) {
	assert(valid_stepper_io(&self->io));
	// TODO: Add initialization, setup GPIO
	self->is_simplified = false;

	GPIO_InitTypeDef init;
	init.Pin = 
		self->io.dir | self->io.enable | self->io.reset | self->io.step |
		self->io.ms1 | self->io.ms2 | self->io.ms3;
	init.Mode = MODE_OUTPUT;

	HAL_GPIO_Init(self->io.gpio, &init);

	HAL_GPIO_WritePin(self->io.gpio, self->io.enable, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(self->io.gpio, self->io.reset, GPIO_PIN_RESET);
	
}

void Stepper_init_simplified(Stepper *self) {
	assert(valid_stepper_simplified_io(&self->io));
	self->is_simplified = true;

	GPIO_InitTypeDef init;
	init.Pin = self->io.dir | self->io.step;
	init.Mode = MODE_OUTPUT;

	HAL_GPIO_Init(self->io.gpio, &init);
}

void Stepper_move_to(Stepper *self, uint32_t position) {
	bool is_lesser = self->position < position;
	// Move down if it is, move up if it's not
	// TODO: Test this
	StepperDirection dir = (is_lesser) ? STEPD_BACKWARDS : STEPD_FORWARDS;

	// We cast these to int32_t so that the subtraction is signed
	uint32_t diff = abs((int32_t)self->position - (int32_t)position);
	uint32_t num_steps = diff / um_per_step;

	// Queue up the required number of steps
	// This assert shouldn't be necessary, but it also shouldn't be not necessary
	assert(Stepper_queue_steps(self, num_steps, dir));
}

void Stepper_move_to_immediate(Stepper* self, uint32_t position) {
	bool is_lesser = self->position < position;
	// Move down if it is, move up if it's not
	// TODO: Test this
	StepperDirection dir = (is_lesser) ? STEPD_COUNTERCLOCKWISE : STEPD_CLOCKWISE;
	Stepper_set_direction(self, dir);

	// We cast these to int32_t so that the subtraction is signed
	uint32_t diff = abs((int32_t)self->position - (int32_t)position);
	uint32_t num_steps = diff / um_per_step;

	for (int i = 0; i < num_steps; i++) {
		Stepper_step_immediate(self);
	}
}

bool Stepper_queue_steps(Stepper *self, uint32_t step_count, StepperDirection dir) {
	// Reset the step pin
	HAL_GPIO_WritePin(self->io.gpio, self->io.step, GPIO_PIN_RESET);

	for (int i = 0; i < MAX_STEPPERS; i++) {
		// Find `self` in the queue, or the first empty entry
		if (stepper_queue[i].stepper == self || stepper_queue[i].num_steps == 0 || stepper_queue[i].stepper == NULL) {
			// Update the entry
			stepper_queue[i] = (StepperQueueEntry) {
				.stepper = self, .num_steps = step_count, .dir = dir, .is_high = false
			};
			return true;
		}
	}
	return false;
}

#pragma endregion

#pragma region StepperQueue

void StepperQueueEntry_operate(StepperQueueEntry* entry) {
	Stepper* stepper = entry->stepper;
	if (stepper && entry->num_steps > 0) {
		// Set the direction if it's somehow been changed
		if (entry->dir != stepper->dir) {
			Stepper_set_direction(stepper, entry->dir);
		}
		// We decrement the count upon setting the value low.
		// If the pin is high then set it low and decrement the number of steps
		if (entry->is_high) {
			HAL_GPIO_WritePin(stepper->io.gpio, stepper->io.step, GPIO_PIN_RESET);
			entry->is_high = false;
			entry->num_steps -= 1;
			// Update the position since the stepper has changed
			stepper->position += (stepper->dir == STEPD_FORWARDS) ? um_per_step : -um_per_step;
		}
		// Otherwise, set it high and wait for the next call
		else {
			HAL_GPIO_WritePin(stepper->io.gpio, stepper->io.step, GPIO_PIN_SET);
			entry->is_high = true;
		}
	}
}

void StepperQueue_iterate() {
	for (int i = 0; i < MAX_STEPPERS; i++) {
		StepperQueueEntry_operate(&stepper_queue[i]);
	}
}

bool StepperQueue_has(Stepper *s) {
	for (int i = 0; i < MAX_STEPPERS; i++) {
		StepperQueueEntry se = stepper_queue[i];
		if (se.stepper == s && se.num_steps > 0) {
			return true;
		}
	}
	return false;
}
#pragma endregion
#pragma endregion