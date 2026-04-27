
#include "servo.h"
#include "common.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include <stdint.h>
// #include <math.h>

void PCA9685_write(PCA9685 *board, uint8_t reg, uint8_t data) {
  uint8_t args[] = {reg, data};
  I2C_Write_many(board->i2c, board->addr, args, 2);
}

void PCA9685_sleep(PCA9685 *board) {
  PCA9685_write(board, MODE1_REG, 0x10);
  HAL_Delay(10);
}

void PCA9685_set_prescaler(PCA9685 *board, uint8_t prescaler) {
  board->prescaler = prescaler;
  PCA9685_write(board, PRESCALER_REG, prescaler);
  HAL_Delay(10);
}

void PCA9685_Init(PCA9685 *board) {
  // TODO?: Remove magic numbers, make everything easier to understand
  PCA9685_sleep(board); // Set board to sleep
  PCA9685_set_prescaler(
      board, (board->prescaler)
                 ? board->prescaler
                 : 121); // Set prescaler (default: 50MHz TODO: Calculate)

  PCA9685_write(board, MODE1_REG, 0x21); // Wake and auto-increment flags
  HAL_Delay(10);
  PCA9685_write(board, MODE2_REG, 0x04); // Totem pole (?)
  HAL_Delay(10);
}

// Calculates the servo's value based on its `range_min` and `range_max` using
// the target (between `SERVO_MIN` and `SERVO_MAX`)
uint16_t _Servo_lerp(uint16_t a, uint16_t b, float t) {
  t = (t - SERVO_MIN) / (SERVO_MAX - SERVO_MIN);
	t = 1.0 - t;	// Invert t because we're mapping in an inverse range
  return a + (uint16_t)(t * (float)(b - a));
}

void Servo_set_value(Servo *servo, float value) {
  // Calculate the base register for this servo
  uint8_t base_register = PWM_REGISTER(servo->channel);

  // Calculate the absolute value from the target in radians
  uint16_t value_u = _Servo_lerp(servo->range_min, servo->range_max, value);

  PCA9685_write(servo->board, base_register, 0);
  PCA9685_write(servo->board, base_register + 1, 0);
  PCA9685_write(servo->board, base_register + 2, value_u & 0xFF);
  PCA9685_write(servo->board, base_register + 3, (value_u >> 8) & 0xFF);
}

void Servo_set_value_direct(Servo *servo, uint16_t value) {
  uint8_t base_register = PWM_REGISTER(servo->channel);

  PCA9685_write(servo->board, base_register, 0);
  PCA9685_write(servo->board, base_register + 1, 0);
  PCA9685_write(servo->board, base_register + 2, value & 0xFF);
  PCA9685_write(servo->board, base_register + 3, (value >> 8) & 0xFF);
}

void Servo_update(Servo *servo) {
  if (servo->value > SERVO_MAX)
    servo->value = SERVO_MAX;
  if (servo->value < SERVO_MIN)
    servo->value = SERVO_MIN;
  Servo_set_value(servo, servo->value);
}

bool Servo_rotate_delta(Servo *servo, float delta) {
  // Figure out the difference and rotate it towards the target by the delta
  if (Servo_at_destination(servo))
    return true;
  float diff = servo->target - servo->value;
  if (diff < 0) {
    diff = -diff;
    delta = (delta < diff) ? delta : diff;
    servo->value -= delta;
  } else if (diff > 0) {
    delta = (delta < diff) ? delta : diff;
    servo->value += delta;
  }
  Servo_update(servo);
  return Servo_at_destination(servo);
}

void Servo_set_target(Servo *servo, float target) { servo->target = target; }

bool Servo_at_destination(Servo *servo) {
  if (fabsf(servo->target - servo->value) < 0.00001) {
    servo->value = servo->target;
    return true;
  }
  return false;
}