#include "estop.h"
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
PCA9685 board = {.addr = 0x40, .prescaler = 121};

// This also triggers when something goes wrong with a stepper or servo.
// We don't know why, but it just does.
bool ESTOP_Callback() {
  USART_write_string(USB_USART, "!err:ESTOP;\n");
  PCA9685_sleep(&board);
  return false;
}

// The probes and their associated I/O
ProbeSet probes = {
    .left =
        {.rail =
             {
                 .io = {.gpio = GPIOB, .step = GPIO_PIN_2, .dir = GPIO_PIN_3},
             },
         .axis = {.board = &board,
                  .channel = 0,
                  .range_max = 430,
                  .range_min = 185},
         .side = Left,
         .io = {.gpio = GPIOC,
                .probe_pin = GPIO_PIN_5,
                .homing_pin = GPIO_PIN_6}},
    .right =
        {.rail = {.io = {.gpio = GPIOB, .step = GPIO_PIN_0, .dir = GPIO_PIN_1}},
         .axis = {.board = &board,
                  .channel = 1,
                  .range_max = 430,
                  .range_min = 185},
         .side = Right,
         .io = {.gpio = GPIOC, .probe_pin = 0, .homing_pin = GPIO_PIN_8}},
    .bed = {// TODO: Other GPIO
            .stepper = {.io = {.gpio = GPIOB,
                               .step = GPIO_PIN_4,
                               .dir = GPIO_PIN_5}}

    }};

int main(void) {
  // Init Hardware Abstraction Layer
  HAL_Init();
  // Enable GPIOB and GPIOC
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  // Init LED for lights and stuff :)
  LED_Init();
  // Init estop pin
  ESTOP_init();

  // Do a delay before working any further (fixes something, I forget what.)
  HAL_Delay(1000);

  // Initialize USB serial, which also initializes the stdout and stdin
  USB_init();

  // Initialize I2C1
  board.i2c = I2C1_Init();

  // Initialize probe set
  ProbeSet_init(&probes);

  // TODO: Finalize main loop implementation

  printf("!dbg:Ready;\n");

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
