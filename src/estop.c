#include "estop.h"
#include "stm32f4xx.h"

void ESTOP_init() {
  __disable_irq();
  {

    RCC->AHB1ENR |= 0x04;
    RCC->APB2ENR |= 0x4000;
    GPIOA->MODER &= ~(0x3 << 12);
    GPIOA->MODER |= (0x1 << 12);
    GPIOC->MODER &= ~(0x3 << 20);
    GPIOC->PUPDR &= ~(0x3 << 20);
    GPIOC->PUPDR |= (0x1 << 20);
    SYSCFG->EXTICR[2] &= ~0x0F00;
    SYSCFG->EXTICR[2] |= 0x0200;
    EXTI->IMR |= 0x0400;
    EXTI->FTSR |= 0x0400;
    NVIC_EnableIRQ(EXTI15_10_IRQn);
  }
  __enable_irq();
}

void EXTI15_10_IRQHandler() {
  EXTI->PR = 0x0400;
	GPIOA->ODR ^= (1 << 6);

	if (ESTOP_Callback) {
		// If the estop is recoverable, then recover!
		if (ESTOP_Callback()) {
			return;
		}
		while (true) {}
	} else {
		// We should panic if no callback is defined and we get an estop!
		while (true) {}
	}
}