#include "gcodes.h"
#include "stm32f446xx.h"
#include "stm32f4xx.h"

#include "stepper.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "usart.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>

#define LED_PIN                                GPIO_PIN_5
#define LED_GPIO_PORT                          GPIOA
#define LED_GPIO_CLK_ENABLE()                  __HAL_RCC_GPIOA_CLK_ENABLE()

void LED_Init();

Stepper test_stepper;

void __gcode_assert(const char * filename, int line, const char * funcname, const char * what_broke) {
	printf("Exception in file: \"%s:%d\": \n\tFunction \"%s\": `%s` evaluated to false", filename, line, funcname, what_broke);
}

void __assert_func(const char * filename, int line, const char * funcname, const char * what_broke) {
	printf("Exception in file: \"%s:%d\": \n\tFunction \"%s\": `%s` evaluated to false", filename, line, funcname, what_broke);
	while (true) {} // Loop forever
}

// Placeholder gcode execution function
void exec_gcode(Gcode gcode) {
	gcode_assert(gcode.num_args >= 1);
	// Switch based on command group (this is the letter of the command)
	switch (gcode.args[0].id) {
		case 'R': {
			// Switch based on command id (this is the number of the command)
			switch (gcode.args[0].value) {
				/*  Command R20:
				 *    Commands R20 R{N} and R20 L{N} rotate the stepper motor by N steps clockwise
				 *    and counterclockwise respectfully.
				 */
				case 20: {
					// Assert that the input is valid. Panic if not.
					gcode_assert(gcode.num_args >= 2);
					char id = gcode.args[1].id;
					gcode_assert(id == 'L' || id == 'R');

					// Get the direction based on the argument id, and the number of turns
					// based on the argument value
					StepperDirection dir = (id == 'L') ? STEPD_COUNTERCLOCKWISE : STEPD_CLOCKWISE;
					int num_turns = gcode.args[1].value;

					// Alert the user over USART stdout
					printf("Turning %s %d ticks...\n", (dir == STEPD_CLOCKWISE) ? "clockwise" : "counterclockwise",  num_turns);
					fflush(stdout);
					// Set the direction of the stepper
					Stepper_set_direction(&test_stepper, dir);
					// Actually rotate the stepper
					for (int i = 0; i < num_turns; i++) {
						Stepper_step(&test_stepper);
					}
				} break;
				default: {
					printf("Unrecognized gcode instruction: %c%d", gcode.args[0].id, gcode.args[0].value);
				} break;
			}
		} break;
		case 'G': {
			switch (gcode.args[0].value) {
				/*  Command G00:
				 *    Simple debug command that toggles the on-board LED
				 */
				case 0: {
					HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
				} break;
			}
		} break;
		default: {
			printf("Unrecognized gcode instruction: %c%d", gcode.args[0].id, gcode.args[0].value);
		} break;
	}
}

int main(void) {
	// Init Hardware Abstraction Layer
  HAL_Init();
	// Enable GPIOB
	__HAL_RCC_GPIOB_CLK_ENABLE();
	// Init LED for lights and stuff :)
  LED_Init();

	// Do a delay before working any further (fixes something, I forget what.)
	HAL_Delay(1000);

	// Initialize USB serial, which also initializes the stdout and stdin
	USB_init();

	test_stepper.io = (StepperIO){
		.gpio = GPIOB, .step = GPIO_PIN_13, .dir = GPIO_PIN_14
	};
	Stepper_init_simplified(&test_stepper);

	HAL_Delay(100);

	Stepper_set_direction(&test_stepper, STEPD_CLOCKWISE);

	// Prompt the user for a gcode instruction, and then execute it.
	while (true) {
		printf("Enter an instruction: ");

		// Decode the next valid instruction and then flush the USART
		Gcode test = gcode_decode(stdin);
		USART_flush(USB_USART);

		// Print the commmand we got
		printf("Got %d args: ", test.num_args);
		for (int i = 0; i < test.num_args; i++) {
			printf("%c%d ", test.args[i].id, test.args[i].value);
		}
		printf("\n");
		// Run the command
		exec_gcode(test);
	}

  while (1)
  {
		// Print to stdout and flush it
		// printf("Hello world! %d\n", i++);
		fflush(stdout);
		
		// Toggle onboard LED
		GPIOA->ODR |= 0x0020;
    HAL_Delay(1000);
		GPIOA->ODR &= ~0x0020;
		HAL_Delay(1000);
  }
}

void LED_Init() {
  LED_GPIO_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
}

void SysTick_Handler(void) {
  HAL_IncTick();
}
