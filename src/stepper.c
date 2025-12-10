#include "stepper.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include <assert.h>



void Stepper_step(Stepper* self) {
	HAL_GPIO_TogglePin(self->io.gpio, self->io.step);

	// TODO: Make this delay shorter somehow. Maybe add a custom timer implementation?
	HAL_Delay(1);

	HAL_GPIO_TogglePin(self->io.gpio, self->io.step);
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
	self->is_simplified = true;

	GPIO_InitTypeDef init;
	init.Pin = self->io.dir | self->io.step;
	init.Mode = MODE_OUTPUT;

	HAL_GPIO_Init(self->io.gpio, &init);
}