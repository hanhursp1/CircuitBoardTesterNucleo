/***************************************************
* Lab_ / usart2.h
*
* Header containing definitions of usart functions
* for USART2
*
*
* Author: Philip Hanhurst
* Date Last Modified: 2/21/2025
*
***************************************************/

// Only import this file once
#pragma once
// #include "stm32f446xx.h"
#include "stm32f4xx.h"
#include "common.h"
#include <stdint.h>
#include <stdio.h>

#define USB_USART USART2

typedef USART_TypeDef* USART;

USART USB_init();
void USART_write(USART u, uint8_t c);
char USART_read(USART u);

bool USART_has_data(USART u);
void USART_wait_for_data(USART u);

int USART_write_string(USART u, const char* str);
int USART_read_string(USART u, char* buffer, int bufferlen);

FILE* USART_fopen(USART u);
#define DBG(s) USART_write_string(USB_USART, s)