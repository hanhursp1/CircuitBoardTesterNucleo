#include "gcodes.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"

#include "stepper.h"

#include "usart.h"
#include <stdio.h>

#define LED_PIN                                GPIO_PIN_5
#define LED_GPIO_PORT                          GPIOA
#define LED_GPIO_CLK_ENABLE()                  __HAL_RCC_GPIOA_CLK_ENABLE()

void LED_Init();

// TODO: Get scanf working

int main(void) {
  HAL_Init();
  LED_Init();

	// Do a delay before working any further (fixes something, I forget what.)
	HAL_Delay(1000);

	// Initialize USB serial, which also initializes the stdout
	USB_init();

	HAL_Delay(100);
	printf("Enter an argument: ");

	GcodeArg testArg = gcode_arg_decode(stdin);

	printf("Got: %c%d\n", testArg.id, testArg.value);

	int i = 0;

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
