#include "probe.h"
#include "common.h"
#include "config.h"
#include "i2c.h"
#include "servo.h"
#include "stepper.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include <math.h>

void ProbeSet_init(ProbeSet *probes) {
  // Initialize bed
  {
    // Init homing pin
    GPIO_InitTypeDef bed_init = {
        .Pin = probes->bed.homing,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_PULLUP,
    };
    HAL_GPIO_Init(probes->bed.gpio, &bed_init);

    // Init stepper
    Stepper_init_simplified(&probes->bed.stepper);
  }
  // Initialize left and right probe steppers
  Stepper_init_simplified(&probes->left.rail);
  Stepper_init_simplified(&probes->right.rail);
  // Initialize servo board (assume left and right share the same board)
  if (probes->left.axis.board != probes->right.axis.board) {
    // TODO: Panic or something I guess.
    // for now, just accomodate this edge case
    PCA9685_Init(probes->left.axis.board);
    PCA9685_Init(probes->right.axis.board);
  } else {
    PCA9685_Init(probes->left.axis.board);
  }

  // Initialize probe IO
  {
    GPIO_InitTypeDef init_left = {.Pin = probes->left.io.homing_pin |
                                         probes->left.io.probe_pin,
                                  .Mode = GPIO_MODE_INPUT,
                                  .Pull = GPIO_PULLUP};
    HAL_GPIO_Init(probes->left.io.gpio, &init_left);

    GPIO_InitTypeDef init_right = {.Pin = probes->right.io.homing_pin |
                                          probes->right.io.probe_pin,
                                   .Mode = GPIO_MODE_INPUT,
                                   .Pull = GPIO_PULLUP};
    HAL_GPIO_Init(probes->right.io.gpio, &init_right);
  }
}

ProbePosition Probe_calculate_position(Probe *probe, uint32_t x, uint32_t y) {
  ProbePosition result;
  // TODO: Test if this works

	switch(probe->side) {
		case Left: {

		} break;
		case Right: {

		} break;
	}

  // Return position
  return result;
}

void Probe_to_position_pair(Probe *probe, ProbePosition position) {
  // TODO: Test this
  // Cache the axis and rail from the probe
  Servo *axis = &probe->axis;
  Stepper *rail = &probe->rail;
  // Move the stepper to its proper location
  Stepper_move_to_immediate(rail, position.position);
  // Set the servo's target position
  Servo_set_target(axis, position.rotation);
  // Update the servo rotation at 20ms tick intervals
  while (!Servo_at_destination(axis)) {
    Servo_rotate_delta(axis, 0.02);
    HAL_Delay(20);
  }
}

void Probe_set_position(Probe *probe, uint32_t x, uint32_t y) {
  // TODO: Test this
  probe->x = x;
  probe->y = y;
  ProbePosition p = Probe_calculate_position(probe, x, y);
  Probe_to_position_pair(probe, p);
}

bool ProbeSet_test_continuity(ProbeSet *probes) {
  // Check which probe is the continuity pin by which has a non-zero probe_pin
  Probe *probe = (probes->right.io.probe_pin) ? &probes->right : &probes->left;
  return HAL_GPIO_ReadPin(probe->io.gpio, probe->io.probe_pin) == 1;
}

void ProbeSet_lower_bed(ProbeSet *probes) {
  Stepper_move_to_immediate(&probes->bed.stepper, 0);
}

void ProbeSet_raise_bed(ProbeSet *probes) {
  Stepper_move_to_immediate(&probes->bed.stepper, BED_HIGH_UM);
}

Stepper *ProbeSet_get_stepper_by_id(ProbeSet *probes, int id) {
  switch (id) {
  case 0:
    return &probes->right.rail;
  case 1:
    return &probes->left.rail;
  case 2:
    return &probes->bed.stepper;
  default:
    return NULL;
  }
}
Servo *ProbeSet_get_servo_by_id(ProbeSet *probes, int id) {
  switch (id) {
  case 0:
    return &probes->right.axis;
  case 1:
    return &probes->left.axis;
  default:
    return NULL;
  }
}