#include "probe.h"
#include "config.h"
#include "servo.h"
#include "stepper.h"
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

ProbePosition Probe_calculate_position(Side side, uint32_t x, uint32_t y) {
  ProbePosition result;
  // TODO: Test if this works
  // Calculate angle from requested X position
  float arm_x_rel = ((float)x + (float)RAIL_OFFSET_L) / (float)PROBE_LEN;
  result.rotation = acosf(arm_x_rel);
  // Calculate relative servo offset from requested X position
  float arm_y_rel = sinf(result.rotation);
  // Calculate absolute servo offset from requested Y position
  result.position = y - (PROBE_LEN * arm_y_rel);
  // Return position
  return result;
}

bool ProbeSet_test_continuity(ProbeSet *probes) {
  // Assume the left probe is connected to vdd
  return HAL_GPIO_ReadPin(probes->right.io.gpio, probes->right.io.probe_pin) ==
         1;
}

void ProbeSet_lower_bed(ProbeSet *probes) {
  Stepper_move_to(&probes->bed.stepper, 0);
}

void ProbeSet_raise_bed(ProbeSet *probes) {
  Stepper_move_to(&probes->bed.stepper, BED_HIGH_UM);
}