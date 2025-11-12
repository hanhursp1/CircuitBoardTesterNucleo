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

	HAL_Delay(1000);

	USB_init();

	int i = 0;
	// scanf("%d", &i);

  while (1)
  {
		printf("Hello world! %d\n", i++);
		fflush(stdout);
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
