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
#include <math.h>
#include <stdio.h>
#include <string.h>

void can_you_repeat_that(FILE *f) { fprintf(f, "!repeat:;"); }

// TODO: Add commments to instructions that need them

DEF_INSTR(vertcnt) {
  int cnt, hash;
  int found = fscanf(f, "%d:%x", &cnt, &hash);
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
  int found = fscanf(f, "%d:%x", &cnt, &hash);
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
  int found = fscanf(f, "%04d:%d,%d:%x", &id, &x, &y, &hash);
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
  int found = fscanf(f, "%04d:%d,%d:%x", &id, &x, &y, &hash);
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

DEF_INSTR(movprobe) {
  int probeid;
  int x, y;
  fscanf(f, "%d:%d,%d", &probeid, &x, &y);
  Probe *probe = (probeid) ? &probes.left : &probes.right;
  // Side side = (probeid) ? Right : Left;
  Probe_set_position(probe, x, y);
  return 0;
}

DEF_INSTR(probepos) {
  int probeid;
  fscanf(f, "%d", &probeid);
  Probe *probe = (probeid) ? &probes.left : &probes.right;
  fprintf(f, "!ok:%d,%d;\n", probe->x, probe->y);
  return 1;
}

// Returns servo rotation, in radians
DEF_INSTR(servrot) {
  int probeid;
  fscanf(f, "%d", &probeid);
  Probe *probe = (probeid) ? &probes.left : &probes.right;
  float proberot = probe->axis.value;
  int whole = floorf(proberot);
  proberot -= whole;
  int fract = floorf(proberot * 100.0);
  fprintf(f, "!ok:%d.%02d;", whole, fract);
  return 1;
}

DEF_INSTR(stepper) {
  int stepid;
  int pos;
  volatile int found = fscanf(f, "%d:%d", &stepid, &pos);
  volatile Stepper *stepper = ProbeSet_get_stepper_by_id(&probes, stepid);
	assert(stepper);
  Stepper_move_to_immediate(stepper, pos);
  return 0;
}

DEF_INSTR(stepperdirect) {
  int id, steps;
  volatile int found = fscanf(f, "%d:%d", &id, &steps);
  // assert(found == 2);
  Stepper *stepper = ProbeSet_get_stepper_by_id(&probes, id);
	assert(stepper);
  StepperDirection dir = STEPD_FORWARDS;
  if (steps < 0) {
    dir = STEPD_BACKWARDS;
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
  int found = fscanf(f, "%d:%d", &id, &deg);
  // assert(found == 2);
  Servo *servo = (id) ? &probes.left.axis : &probes.right.axis;
  float rad = ((float)deg / 360.0) * 2 * PI;
  uint16_t step = _Servo_lerp(servo->range_min, servo->range_max, rad);
  printf("!dbg:%d:%.2f:%hd;\n", id, rad, step);
  servo->target = rad;
  while (!Servo_rotate_delta(servo, 0.005)) {
    printf("!dbg:%d:%d;\n", (int)(1000 * servo->value),
           (int)(1000 * servo->target));
		fflush(stdout);
    HAL_Delay(10);
  };
  return 0;
}

DEF_INSTR(servodirect) {
  int id;
  int val;
  int found = fscanf(f, "%d:%d", &id, &val);
  // assert(found == 2);
  Servo *servo = (id) ? &probes.left.axis : &probes.right.axis;
  Servo_set_value_direct(servo, val);
  return 0;
}

DEF_INSTR(led) {
  // fscanf(f, ";");
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  printf("!dbg:LED Toggled!;\n");
  return 0;
}

const Instruction *const instructions[] = {
    &INSTR(vertcnt),       &INSTR(netcnt),   &INSTR(vert),
    &INSTR(net),           &INSTR(echo),     &INSTR(stepper),
    &INSTR(stepperdirect), &INSTR(servo),    &INSTR(servodirect),
    &INSTR(led),           &INSTR(movprobe), &INSTR(servrot),
    &INSTR(probepos)};

#define INSTR_COUNT (sizeof(instructions) / sizeof(Instruction *))

int execute_instruction(FILE *f) {
  char instr[128] = {0};

  // Try and get to the command field start ('!').
  // This is hacky and uses the USART directly because
  // fgetc kept stalling
  while (USART_read(USB_USART) != '!')
    ;

  // Read instruction from file stream
  fscanf(f, "%[^\n\r:;]:", instr);
  // DEBUG: echo instruction over the UART
  DBG(instr);

  for (int i = 0; i < INSTR_COUNT; i++) {
    // Assume an instruction is valid if it and its callback are not null
    // If the ID matches, execute the instruction, passing the file stream
    if (instructions[i] && instructions[i]->callback &&
        !strcmp(instructions[i]->id, instr)) {
      int result = instructions[i]->callback(f);
      fscanf(f, ";");
      switch (result) {
        // Result returned without issues, print OK response
      case 0: {
        fprintf(f, "!ok;");
      } break;
        // Something happened, print generic error
      case -1: {
        fprintf(f, "!err;");
      } break;
      // We already returned an OK response with something in it, so just break
      case 1: break;
      // We already returned an Err response with something in it, so just break
      case -2: break;
      }
      return result;
    }
  }
  can_you_repeat_that(f);
  return -1;
}