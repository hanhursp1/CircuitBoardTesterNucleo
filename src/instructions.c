#include "instructions.h"
#include "common.h"
#include "config.h"
#include "probe.h"
#include "servo.h"
#include "stepper.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "usart.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void can_you_repeat_that(FILE *f) { fprintf(f, "!repeat:;"); }

DEF_INSTR(vertcnt) {
  int cnt, hash;
  int found = fscanf(f, "%d:%x;", &cnt, &hash);
  if (found < 1) {
    can_you_repeat_that(f);
    return -1;
  } else if (found == 2) {
    // TODO: Check hash
  }
  // TODO: set vert count info
  return 0;
}

DEF_INSTR(netcnt) {
  int cnt, hash;
  int found = fscanf(f, "%d:%x;", &cnt, &hash);
  if (found < 1) {
    can_you_repeat_that(f);
    return -1;
  } else if (found == 2) {
    // TODO: Check hash
  }
  // TODO set net count info
  return 0;
}

DEF_INSTR(vert) {
  int id, x, y, hash;
  int found = fscanf(f, "%04d:%d,%d:%x;", &id, &x, &y, &hash);
  if (found < 3) {
    /* TODO: Panic */
    can_you_repeat_that(f);
    return -1;
  }
  // TODO: Set vertex info
  return 0;
}

DEF_INSTR(net) {
  int id, x, y, hash;
  int found = fscanf(f, "%04d:%d,%d:%x;", &id, &x, &y, &hash);
  if (found < 3) {
    /* TODO: Panic */
    can_you_repeat_that(f);
    return -1;
  }
  // TODO: Set net info
  return 0;
}

DEF_INSTR(echo) {
  // TODO: Print out all vertex and index info to the client
  return 0;
}

DEF_INSTR(run) {
  // TODO: Actually start a run
  return 0;
}

DEF_INSTR(stepper) {
  int id, steps;
  int found = fscanf(f, "%d:%d;", &id, &steps);
  assert(found == 2);
  Stepper *stepper = (id) ? &probes.left.rail : &probes.right.rail;
  StepperDirection dir = STEPD_CLOCKWISE;
  if (steps < 0) {
    dir = STEPD_COUNTERCLOCKWISE;
    steps = -steps;
  }
  Stepper_set_direction(stepper, dir);

  while (steps--) {
    Stepper_step_immediate(stepper);
  }
  return 0;
}

DEF_INSTR(servo) {
  int id;
  int deg;
  int found = fscanf(f, "%d:%d;", &id, &deg);
  // assert(found == 2);
  Servo *servo = (id) ? &probes.left.axis : &probes.right.axis;
  float rad = ((float)deg / 360.0) * 2 * PI;
	uint16_t step = _Servo_lerp(servo->range_min, servo->range_max, rad);
	printf("!dbg:%d:%.2f:%hd;\n", id, rad, step);
  servo->target = rad;
  while (!Servo_rotate_delta(servo, 0.005)) {
		printf("!dbg:%d:%d;\n", (int)(1000 * servo->value), (int)(1000 * servo->target));
    HAL_Delay(10);
  };
  return 0;
}

DEF_INSTR(led) {
  fscanf(f, ";");
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	printf("!dbg:LED Toggled!;\n");
  return 0;
}

const Instruction *const instructions[] = {
    &INSTR(vertcnt), &INSTR(netcnt),  &INSTR(vert),  &INSTR(net),
    &INSTR(echo),    &INSTR(stepper), &INSTR(servo), &INSTR(led)};

#define INSTR_COUNT (sizeof(instructions) / sizeof(Instruction *))

int execute_instruction(FILE *f) {
  char instr[128];
	
	while (fgetc(f) != '!') {}
	// printf("Parsing command: ");
	char c;
	int i = 0;
	while ((c = fgetc(f)) != ':') {
		instr[i++] = c;
	}

  // if (FETCH_INSTR(f, instr) < 1) {
  //   // Failed to fetch an instruction, do something
	// 	printf("Failed to fetch instruction!");
  //   return -1;
  // }
	// printf("!dbg:%s;\n", instr);
  for (int i = 0; i < INSTR_COUNT; i++) {
    // Assume an instruction is valid if its callback is not null
    // If the ID matches, execute the instruction, passing the file stream
    if (instructions[i]->callback && !strcmp(instructions[i]->id, instr)) {
      int result = instructions[i]->callback(f);
			switch (result) {
				case 0: {
					printf("!ok:;");
				} break;
				case -1: {
					printf("!err:;");
				}
			}
			return result;
    }
  }
  can_you_repeat_that(f);
  return -1;
}