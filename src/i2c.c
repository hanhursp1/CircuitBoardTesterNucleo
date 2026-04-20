
#include "i2c.h"
#include "stm32f446xx.h"
#include "stm32f4xx.h"
#include <stddef.h>
#include <stdint.h>

// Initialize I2C1 (PB8 = SCL, PB9 = SDA)
I2C I2C1_Init(void) {
  GPIOB->MODER &= ~0x000F0000;
  GPIOB->MODER |= 0x000A0000;

  GPIOB->OTYPER |= 0x00000300;  // Open-drain
  GPIOB->OSPEEDR |= 0x000F0000; // High speed

  GPIOB->PUPDR &= ~0x000F0000;
  GPIOB->PUPDR |= 0x00050000; // Pull-up resistors

  GPIOB->AFR[1] &= ~0x000000FF;
  GPIOB->AFR[1] |= 0x00000044; // AF4 (I2C)

  I2C1->CR1 |= 0x8000; // Software reset
  I2C1->CR1 &= ~0x8000;

  I2C1->CR2 = 16; // Peripheral clock = 16 MHz
  I2C1->CCR = 80; // 100 kHz
  I2C1->TRISE = 17;

  I2C1->CR1 |= 0x0001; // Enable I2C1

  return I2C1;
}

// Write one byte to an I2C device register
void I2C_Write(I2C i2c, uint8_t addr, uint8_t data) {
  i2c->CR1 |= 0x0100; // START
  while (!(i2c->SR1 & 1))
    ;

  i2c->DR = addr << 1; // Address + write
  while (!(i2c->SR1 & 2))
    ;
  (void)i2c->SR2;

  // Write the data byte to the i2c interface data register
  while (!(i2c->SR1 & 0x80))
    ;
  i2c->DR = data;

  while (!(i2c->SR1 & 4))
    ;

  i2c->CR1 |= 0x0200; // STOP
}

void I2C_Write_many(I2C i2c, uint8_t addr, uint8_t data[], size_t len) {
  i2c->CR1 |= 0x0100; // START
  while (!(i2c->SR1 & 1))
    ;

  i2c->DR = addr << 1; // Address + write
  while (!(i2c->SR1 & 2))
    ;
  (void)i2c->SR2;

  // Write every data byte to the i2c interface data register
  while (len--) {
    while (!(i2c->SR1 & 0x80))
      ;
    i2c->DR = *(data++);
  }

  while (!(i2c->SR1 & 4))
    ;

  i2c->CR1 |= 0x0200; // STOP
}