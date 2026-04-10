#pragma once

#include "common.h"
#include "i2c.h"
#include <stdint.h>

#define LED0_ON_L       0x06   // Base register for LED0 PWM control

typedef struct PCA9685 {
	I2C i2c;
	uint8_t addr;
	uint8_t prescaler;
} PCA9685;

typedef struct Servo {
	PCA9685* board;
	uint8_t channel;
	uint16_t value;
	uint16_t target;
	uint16_t range_min;
	uint16_t range_max;
} Servo;

void PCA9685_Init(PCA9685* board);
void PCA9685_SetPWM(PCA9685* board, uint8_t ch, uint16_t off);

void Servo_set_value(Servo* servo, uint16_t target);
void Servo_update(Servo* servo);

bool Servo_rotate_delta(Servo* servo, uint16_t delta);

void Servo_set_rotation_degrees(Servo* servo, uint16_t deg);
void Servo_set_rotation_radians(Servo* servo, float rad);