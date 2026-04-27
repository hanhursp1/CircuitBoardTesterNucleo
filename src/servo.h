#pragma once

#include "common.h"
#include "i2c.h"
#include <stdint.h>

#define PWM_REG_BASE       	0x06   // Base register for LED0 PWM control
#define PWM_REGISTER(N)			(PWM_REG_BASE + (4 * N))

#define MODE1_REG 					0x00
#define MODE2_REG 					0x01
#define PRESCALER_REG 			0xFE

// TODO: Actually measure servo min and max angles.
// Our servos are from the range of 0 to 120 degrees. Our calculations work in radians
#define SERVO_MIN						0.0				// Minimum servo range (0 degrees)
#define SERVO_MAX						(PI * (0.5))	// Maximum servo range (approx 120 degrees)

typedef struct PCA9685 {
	I2C i2c;
	uint8_t addr;
	uint8_t prescaler;
} PCA9685;

typedef struct Servo {
	PCA9685* board;
	uint8_t channel;
	float value;
	float target;
	uint16_t range_min;
	uint16_t range_max;
} Servo;

void PCA9685_write(PCA9685 *board, uint8_t reg, uint8_t data);
void PCA9685_sleep(PCA9685 *board);
void PCA9685_set_prescaler(PCA9685 *board, uint8_t prescaler);

void PCA9685_Init(PCA9685* board);
void PCA9685_SetPWM(PCA9685* board, uint8_t ch, uint16_t off);

void Servo_set_value(Servo* servo, float target);
void Servo_set_value_direct(Servo *servo, uint16_t value);
void Servo_set_target(Servo* servo, float target);
void Servo_update(Servo* servo);

bool Servo_rotate_delta(Servo* servo, float delta);
bool Servo_at_destination(Servo* servo);

void Servo_set_rotation_degrees(Servo* servo, float deg);
void Servo_set_rotation_radians(Servo* servo, float rad);

uint16_t _Servo_lerp(uint16_t a, uint16_t b, float t);