#include "gcodes.h"
#include "packet.h"
#include "stm32f446xx.h"
#include "stm32f4xx.h"

#include "stepper.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "usart.h"
#include "netlist.h"
#include "scheduler.h"
#include <assert.h>
#include <stdint.h>
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

#define INT_FROM_STR(s) (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | (s[3])

void command_test(FILE* f) {
	// Get the 4-char long command
	char str_tmp[5];
	fscanf(f, "%4s", str_tmp);
	// Cast to an int to make comparisons easier
	volatile uint32_t cmd = *(uint32_t*)(str_tmp);
	switch(cmd) {
		// EVERYTHING IS BACKWARDS BECAUSE C DOES LONG CHAR CONSTS IN BIG ENDIAN (bad)
		case '_nur': {
			// TODO: Everything
		} break;
		case 'trev': {
			fill_pointlist_from_file(f);
			fflush(f);
			USART_flush(USB_USART);
		} break;
		case 'sten': {
			fill_netlist_from_file(f);
			fflush(f);
			USART_flush(USB_USART);
		} break;
		case 'ohce': {
			// int num_verts = get_vert_count();
			int num_nets = get_net_count();
			for (int i = 0; i < num_nets; i++) {
				printf("Net(%04d) {", i);
				NetlistEntry net = nets_buffer[i];
				for (int j = 0; j < net.length; j++) {
					NetlistPoint point = points_buffer[net.start_index + j];
					printf("%d, %d; ", point.x, point.y);
				}
				printf("} :: ");
			}
			fflush(stdout);
		} break;
		default: {
			// char* c = (char*)&cmd;
			// char str[5] = {c[0], c[1], c[2], c[3], 0};
			char s[1024];
			fscanf(f, "%s", s);
			printf("Invalid command: %s%s", str_tmp, s);
			fflush(f);
			// This unfortunately cannot be abstracted away with the file API
			USART_flush(USB_USART);
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

	crc_init();

	// test_stepper.io = (StepperIO){
	// 	.gpio = GPIOB, .step = GPIO_PIN_13, .dir = GPIO_PIN_14
	// };
	// Stepper_init_simplified(&test_stepper);

	// HAL_Delay(100);

	// Stepper_set_direction(&test_stepper, STEPD_CLOCKWISE);

	// // Prompt the user for a gcode instruction, and then execute it.
	// while (true) {
	// 	printf("Enter an instruction: ");

	// 	// Decode the next valid instruction and then flush the USART
	// 	Gcode test = gcode_decode(stdin);
	// 	USART_flush(USB_USART);

	// 	// Print the commmand we got
	// 	printf("Got %d args: ", test.num_args);
	// 	for (int i = 0; i < test.num_args; i++) {
	// 		printf("%c%d ", test.args[i].id, test.args[i].value);
	// 	}
	// 	printf("\n");
	// 	// Run the command
	// 	exec_gcode(test);
	// }

  while (true)
  {
		fflush(stdin);
		USART_flush(USB_USART);
		command_test(stdin);
		// scheduling_test();
  }
	HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_SET);
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

