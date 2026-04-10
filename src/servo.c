
#include "servo.h"
#include "i2c.h"
#include <stdint.h>
// #include <math.h>

#define PI 3.1415926

void Servo_set_value(Servo* servo, uint16_t target) {
	uint8_t reg = LED0_ON_L + 4 * servo->channel;

	I2C_Write(servo->board->i2c, servo->channel, reg   , 0);
	I2C_Write(servo->board->i2c, servo->channel, reg + 1, 0);
	I2C_Write(servo->board->i2c, servo->channel, reg + 2, target & 0xFF);
	I2C_Write(servo->board->i2c, servo->channel, reg + 3, (target >> 8) & 0xFF);
}

void Servo_update(Servo *servo) {
	Servo_set_value(servo, servo->value);
}

bool Servo_rotate_delta(Servo *servo, uint16_t delta) {
	int16_t diff = servo->target - servo->value;
	if (diff < 0) {
		diff = -diff;
		delta = (delta < diff) ? delta : diff;
		servo->value -= delta;
	} else if (diff > 0) {
		delta = (delta < diff) ? delta : diff;
		servo->value += delta;
	}
	Servo_update(servo);
	return servo->target == servo->value;
}

void Servo_set_rotation_degrees(Servo* servo, uint16_t deg) {
	Servo_set_rotation_radians(servo, ((float)deg / 180.0) * PI);
}

void Servo_set_rotation_radians(Servo* servo, float rad) {
	if (rad < 0) rad = 0.0;
	if (rad > PI) rad = PI;

	servo->target = servo->range_min + ((servo->range_max - servo->range_min) * rad) / PI;
}