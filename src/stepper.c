#include "stepper.h"
#include "stm32f4xx_hal.h"



void Stepper_step(Stepper *self) {
	HAL_GPIO_TogglePin(self->io.gpio, self->io.step);

	// TODO: Add a short wait period. Implement this.
	HAL_Delay(1);

	HAL_GPIO_TogglePin(self->io.gpio, self->io.step);
}

void Stepper_set_mode(Stepper* self, StepperMode mode) {
	self->mode = mode;
	// TODO: update pins

	HAL_GPIO_WritePin(self->io.gpio, self->io.ms1, (mode & 0b001) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(self->io.gpio, self->io.ms2, (mode & 0b010) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(self->io.gpio, self->io.ms3, (mode & 0b100) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Stepper_init(Stepper *self) {
	// TODO: Add initialization, setup GPIO

	GPIO_InitTypeDef init;
	init.Pin = 
		self->io.dir | self->io.enable | self->io.reset | self->io.step |
		self->io.ms1 | self->io.ms2 | self->io.ms3;
	init.Mode = MODE_OUTPUT;

	HAL_GPIO_Init(self->io.gpio, &init);

}