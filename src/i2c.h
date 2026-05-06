#pragma once

#include "common.h"
#include "stm32f446xx.h"
#include <stddef.h>
#include <stdint.h>

typedef I2C_TypeDef* I2C;

I2C I2C1_Init(void);
void I2C_Write(I2C i2c, uint8_t addr, uint8_t data);
void I2C_Write_many(I2C i2c, uint8_t addr, uint8_t data[], size_t len);

