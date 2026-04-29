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
	// Calculate the absolute X position
	const float rail_offset = (probe->side == Right) ? RAIL_OFFSET_R : RAIL_OFFSET_L;
	float probe_x_abs = (probe->side == Right) ? x + rail_offset : PROBE_LEN - x;
	// Normalize it in terms of PROBE_LEN distance
	float probe_x_nomalized = probe_x_abs / (float)PROBE_LEN;
	// Calculate the angle
	float theta = asinf(probe_x_nomalized);
	result.rotation = theta;

	int32_t probe_y = BED_OFFSET_Y - (PROBE_LEN * cosf(theta));
	result.position = probe_y;

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

inline bool Probe_at_home(Probe *probe) {
	// TODO: Test if the homing pin is active high or active low
	return HAL_GPIO_ReadPin(probe->io.gpio, probe->io.homing_pin) == 1;
}

// Home a single probe
void Probe_home(Probe *probe) {
	// Temporarily raise the probe position to bypass limits
	probe->rail.position = RAIL_LEN - 1;
	// Set the direction backwards
	Stepper_set_direction(&probe->rail, STEPD_BACKWARDS);
	// Step the probe backwards while it isn't homed
	while (!Probe_at_home(probe)) {
		Stepper_step_immediate(&probe->rail);
	}
	probe->rail.position = 0;
}

// Optimized homing operation that sets both probes to home at once
void ProbeSet_home(ProbeSet *probes) {
	// Set both servos to 0
	Servo_set_value(&probes->left.axis, 0.0);
	Servo_set_value(&probes->right.axis, 0.0);

	// Set both stepper directions
	Stepper_set_direction(&probes->left.rail, STEPD_BACKWARDS);
	Stepper_set_direction(&probes->right.rail, STEPD_BACKWARDS);

	while (true) {
		// If both probes are homed, break this loop
		if (Probe_at_home(&probes->left) && Probe_at_home(&probes->right)) {
			break;
		}
		// If the left probe isn't homed, move it backwards
		if (!Probe_at_home(&probes->left)) {
			HAL_GPIO_TogglePin(probes->left.io.gpio, probes->left.io.homing_pin);
		}
		// If the right probe isn't homed, move it backwards
		if (!Probe_at_home(&probes->right)) {
			HAL_GPIO_TogglePin(probes->left.io.gpio, probes->left.io.homing_pin);
		}
		// Delay for 1 ms
		HAL_Delay(1);
	}

	// Reset probe position values
	probes->left.rail.position = 0;
	probes->right.rail.position = 0;
}