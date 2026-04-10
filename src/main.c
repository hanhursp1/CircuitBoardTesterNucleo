#include "gcodes.h"
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

Stepper test_stepper;

void __instruction_assert(const char *filename, int line, const char *funcname,
                          const char *what_broke) {
  printf("!Err:Assert:assert=\"%s\":file=\"%s\":line=%d:func=\"%s\";",
         what_broke, filename, line, funcname);
}

// Based on the ANSI assert implementation. I've modified it to simply return
// rather than panic or loop forever.
#define instruction_assert(_thing)                                             \
  if (!(_thing)) {                                                             \
    __instruction_assert(__FILE__, __LINE__, __func__, #_thing);               \
    return;                                                                    \
  }

void __assert_func(const char *filename, int line, const char *funcname,
                   const char *what_broke) {
  printf("Exception in file: \"%s:%d\": \n\tFunction \"%s\": `%s` evaluated to "
         "false",
         filename, line, funcname, what_broke);
  while (true) {
  } // Loop forever
}

#define INT_FROM_STR(s) (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | (s[3])

void can_you_repeat_that(FILE *f) { fprintf(f, "!Err:BadData;"); }

int command_test(FILE *f) {
	// Seek to an exclaimation point so we know that this is the start of an instruction
	while (getc(f) != '!') {}

  // This is an easy vector for a buffer overflow because C posix APIs are like
  // 60 years old. Thankfully, we're not making something important that's going
  // to be used for actual security purposes.
  char cmd[64];
  // Get command name
  fscanf(f, "%s:", cmd);
  // Check which command we got
  if (!strcmp(cmd, "vertcnt") || !strcmp(cmd, "netcnt")) {
    int cnt, hash;
    int found = fscanf(f, "%d:%x;", &cnt, &hash);
    if (found < 1) {
      can_you_repeat_that(f);
      return -1;
    } else if (found == 2) {
			// TODO: Check hash
    }
    if (!strcmp(cmd, "vertcnt")) {
			// Update vert count list

    } else {
			// Update net count list

		}
  } else if (!strcmp(cmd, "vert") || !strcmp(cmd, "net")) {
    // The formatting is the same so we can reuse most of the code
    bool is_vert = !strcmp(cmd, "vert");
    int id, x, y, hash;
    int found = fscanf(f, "%04d:%d,%d:%x;", &id, &x, &y, &hash);
    if (found < 3) {
      /* TODO: Panic */
      can_you_repeat_that(f);
      return -1;
    }
    union {
      NetlistPoint point;
      NetlistEntry entry;
    } item;
    if (is_vert) {
      item.point.x = x;
      item.point.y = y;
    } else {
      item.entry.start_index = x;
      item.entry.length = y;
    }
    if (found == 4) {
      // TODO: Check the data's hash
      can_you_repeat_that(f);
      return -1;
    }
    if (is_vert) {
      points_buffer[id] = item.point;
    } else {
      nets_buffer[id] = item.entry;
    }
  } else if (!strcmp(cmd, "run")) {
		
	}
  return 0;
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

  while (true) {
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

void SysTick_Handler(void) { HAL_IncTick(); }
