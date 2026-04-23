#include "i2c.h"
#include "instructions.h"
#include "probe.h"
#include "servo.h"
#include "stm32f446xx.h"
#include "stm32f4xx.h"

#include "stepper.h"

#include "netlist.h"
#include "scheduler.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "usart.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define LED_PIN GPIO_PIN_5
#define LED_GPIO_PORT GPIOA
#define LED_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

void LED_Init();

// The Servo motor board
PCA9685 board = {
	.addr = 0x40,
	.prescaler = 121
};

// The probes and their associated I/O
// TODO: completely intialize with full GPIO layout
ProbeSet probes = {
	.left = {
		// TODO: GPIO
		.rail = {
			.io = {
				.gpio = GPIOB, .step = GPIO_PIN_13, .dir = GPIO_PIN_14
			},
		},
		.axis = {
			.board = &board, .channel = 0, .range_max = 430, .range_min = 185
		}
	},
	.right = {
		// TODO: Servo, Stepper, GPIO
		.axis = {
			.board = &board, .channel = 1, .range_max = 430, .range_min = 185
		}
	},
	.bed = {
		// TODO: GPIO, Stepper
	}
};

int main(void) {
  // Init Hardware Abstraction Layer
  HAL_Init();
  // Enable GPIOB
  __HAL_RCC_GPIOB_CLK_ENABLE();
  // Init LED for lights and stuff :)
  LED_Init();
	// board.i2c = I2C1_Init();

  // Do a delay before working any further (fixes something, I forget what.)
  HAL_Delay(1000);

  // Initialize USB serial, which also initializes the stdout and stdin
  USB_init();

  // test_stepper.io = (StepperIO){
  // 	.gpio = GPIOB, .step = GPIO_PIN_13, .dir = GPIO_PIN_14
  // };
  // Stepper_init_simplified(&probes.left.rail);
	// board.i2c = I2C1_Init();

	// PCA9685_Init(&board);
	

	printf("Hello world!\n");


  while (true) {
    fflush(stdin);
    USART_flush(USB_USART);
		execute_instruction(stdin);
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

void SysTick_Handler(void) { HAL_IncTick(); }
