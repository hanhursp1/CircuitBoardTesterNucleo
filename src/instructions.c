#include "instructions.h"
#include "common.h"
#include "config.h"
#include "netlist.h"
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

void can_you_repeat_that(FILE *f) {
  HAL_Delay(10);
  fprintf(f, "!repeat:;\n");
}

// TODO: Add commments to instructions that need them

DEF_INSTR(vertcnt) {
  int cnt, hash;
  int found = fscanf(f, "%d", &cnt);
  if (found < 1) {
    can_you_repeat_that(f);
    return -1;
  }
  set_vert_count(cnt);
  // TODO: set vert count info
  return 0;
}

DEF_INSTR(netcnt) {
  int cnt, hash;
  int found = fscanf(f, "%d", &cnt);
  if (found < 1) {
    can_you_repeat_that(f);
    return -1;
  }
  set_net_count(cnt);
  // TODO set net count info
  return 0;
}

DEF_INSTR(vert) {
  int id, x, y, hash;
  int found = fscanf(f, "%d:%d,%d", &id, &x, &y);
  if (found < 3) {
    /* TODO: Panic */
    can_you_repeat_that(f);
    return -1;
  }
  set_vert(id, x, y);
  // TODO: Set vertex info
  return 0;
}

DEF_INSTR(verts) {
  int start, len;
  int found = fscanf(f, "%d,%d", &start, &len);
  if (found < 2) {
    /* TODO: Panic */
    can_you_repeat_that(f);
    return -2;
  }
  for (int i = start; i < start + len; i++) {
    int x, y;
    found = fscanf(f, ":%d,%d", &x, &y);
    if (found < 2) {
      can_you_repeat_that(f);
      return -2;
    }
    set_vert(i, x, y);
  }
  // TODO: Set vertex info
  return 0;
}

DEF_INSTR(net) {
  int id, start, len, hash;
  int found = fscanf(f, "%04d:%d,%d", &id, &start, &len);
  if (found < 3) {
    /* TODO: Panic */
    can_you_repeat_that(f);
    return -1;
  }
  set_net(id, start, len);
  // TODO: Set net info
  return 0;
}

DEF_INSTR(nets) {
  int start, len;
  int found = fscanf(f, "%d,%d", &start, &len);
  if (found < 2) {
    /* TODO: Panic */
    can_you_repeat_that(f);
    return -2;
  }
  for (int i = start; i < start + len; i++) {
    int x, y;
    found = fscanf(f, ":%d,%d", &x, &y);
    if (found < 2) {
      can_you_repeat_that(f);
      return -2;
    }
    set_net(i, x, y);
  }
  // TODO: Set vertex info
  return 0;
}

DEF_INSTR(echo) {
  // TODO: Print out all vertex and index info to the client
  for (int i = 0; i < get_net_count(); i++) {
    NetlistEntry n = get_net(i);
    printf("!dbg:net:%d:%X,%d;\n", i, n.start_index, n.length);
    for (int j = 0; j < n.length; j++) {
      NetlistPoint p = get_point(n.start_index + j);
      printf("!dbg:point:%d:%06d,%06d;\n", j, p.x, p.y);
    }
  }
  return 0;
}

DEF_INSTR(run) {
  // TODO: Actually start a run
  for (int i = 0; i < get_net_count(); i++) {
    HAL_Delay(10);
    NetlistEntry n = get_net(i);
    printf("!dbg:Running Net %d;", i);
    HAL_Delay(5000);
    for (int j = n.start_index; j < n.start_index + n.length; j++) {
      NetlistPoint p = get_point(j);
      HAL_Delay(1000);
      printf("!res:%d:%s;\n", j, "pass");
    }
  }
  HAL_Delay(2000);
  printf("!eof;");
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
  if (!stepper)
    return -1;
  Stepper_move_to_immediate(stepper, pos);
  return 0;
}

DEF_INSTR(stepperdirect) {
  int id, steps;
  volatile int found = fscanf(f, "%d:%d", &id, &steps);
  // assert(found == 2);
  Stepper *stepper = ProbeSet_get_stepper_by_id(&probes, id);
  if (!stepper)
    return -1;
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
  Servo *servo = ProbeSet_get_servo_by_id(&probes, id);
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
  Servo *servo = ProbeSet_get_servo_by_id(&probes, id);
  Servo_set_value_direct(servo, val);
  return 0;
}

DEF_INSTR(servotest) {
  int id;
  int x, y;
  fscanf(f, "%d:%d,%d", &id, &x, &y);
  Probe *probe = (id) ? &probes.left : &probes.right;
  if (!probe)
    return -1;
  volatile ProbePosition pos = Probe_calculate_position(probe, x, y);
  Servo_set_value(&probe->axis, pos.rotation);
  return 0;
}

DEF_INSTR(servotable) {
  Probe *probe = &probes.right;
  uint32_t x = 0;
  while (probe != NULL) {

    // uint32_t x = 0;
    ProbePosition pos = Probe_calculate_position(probe, x, 0);
    while (!isnanf(pos.rotation)) {
      int rot_high = floorf(pos.rotation);
      int rot_low = floorf(pos.rotation * 1000.0) - rot_high;
      printf("!dbg:{side=%s,rot=%d.%d,pos=%d,%d};\n",
             (probe->side == Right) ? "Right" : "Left", rot_high, rot_low,
             x /* + ((probe->side == Right) ? RAIL_OFFSET_R : RAIL_OFFSET_L)*/,
             pos.position);
      x += (probe->side == Right) ? 1000 : -1000;
      pos = Probe_calculate_position(probe, x, 0);
    }
    if (probe == &probes.right)
      probe = &probes.left;
    else
      probe = NULL;
  }
  return 0;
}

DEF_INSTR(ping) {
  HAL_Delay(10);
  printf("!pong;\n");
  fflush(stdout);
  return 1;
}

DEF_INSTR(fakeresp) {
  HAL_Delay(10);
  printf("!ok;\n");
  HAL_Delay(1000);
  for (int i = 0; i < 100; i++) {
    printf("!res:%d:%s;\n", i, (i % 2) ? "pass" : "fail");
    fflush(stdout);
    HAL_Delay(50 * i);
  }
  printf("!eof;\n");
  fflush(stdout);
  return 0;
}

DEF_INSTR(led) {
  // fscanf(f, ";");
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  printf("!dbg:LED Toggled!;\n");
  return 0;
}

const Instruction *const instructions[] = {
    &INSTR(vertcnt),       &INSTR(netcnt),     &INSTR(vert),
    &INSTR(net),           &INSTR(echo),       &INSTR(stepper),
    &INSTR(stepperdirect), &INSTR(servo),      &INSTR(servodirect),
    &INSTR(led),           &INSTR(movprobe),   &INSTR(servrot),
    &INSTR(probepos),      &INSTR(servotest),  &INSTR(ping),
    &INSTR(fakeresp),      &INSTR(servotable), &INSTR(verts),
    &INSTR(nets)};

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
  // DBG(instr);

  for (int i = 0; i < INSTR_COUNT; i++) {
    // Assume an instruction is valid if it and its callback are not null
    // If the ID matches, execute the instruction, passing the file stream
    if (instructions[i] && instructions[i]->callback &&
        !strcmp(instructions[i]->id, instr)) {
      int result = instructions[i]->callback(f);
      fscanf(f, ";");
      HAL_Delay(50);
      switch (result) {
        // Result returned without issues, print OK response
      case 0: {
        USART_write_string(USB_USART, "!ok;\n");
      } break;
        // Something happened, print generic error
      case -1: {
        USART_write_string(USB_USART, "!err;\n");
      } break;
      // We already returned an OK response with something in it, so just break
      case 1:
        break;
      // We already returned an Err response with something in it, so just break
      case -2:
        break;
      }
      fflush(f);
      return result;
    }
  }
  can_you_repeat_that(f);
  return -1;
}