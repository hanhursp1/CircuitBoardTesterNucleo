
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
void USART_flush(USART u);

int USART_write_string(USART u, const char* str);
int USART_read_string(USART u, char* buffer, int bufferlen);

void USART_read_exact(USART u, char* buffer, int len);
void USART_write_exact(USART u, const char* buffer, int len);

FILE* USART_fopen(USART u);

int USART_write_string_debug(USART u, const char* str);
#ifdef DEBUG_ENABLED
#define DBG(s) USART_write_string_debug(USB_USART, s)
#else
#define DBG(s)
#endif