
// required define for cookies
#define _GNU_SOURCE

#include "usart.h"
#include "common.h"
// #include "stm32f446xx.h"
#include "stm32f4xx.h"
// #include <cstdio>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>


USART USB_init() {
	// Enables USART2 over USB
  RCC->AHB1ENR |= 1;       // Enable GPIOA clock
  RCC->APB1ENR |= 0x20000; // Enable USART2 clock

  // Configure PA2 for USART_TX
  GPIOA->AFR[0] &= ~0xFF00;
  GPIOA->AFR[0] |= 0x7700; // alt7 for USART2
  GPIOA->MODER &= ~0x00F0;
  GPIOA->MODER |= 0x00A0; // enable alternate function for PA2 and PA3

  // USART2->BRR = 0x0683;  // 9600 baud @ 16 MHz
	USART2->BRR = UART_BRR_SAMPLING16(HAL_RCC_GetPCLK2Freq(), 115200);
  USART2->CR1 = 0x000C;  // enable Tx and Rx, 8-bit data
  USART2->CR2 = 0x0000;  // 1 stop bit
  USART2->CR3 = 0x0000;  // no flow control
  USART2->CR1 |= 0x2000; // enable USART2

	stdout = USART_fopen(USART2);
	stdin = stdout;

	return USART2;
}

void USART_write(USART u, uint8_t ch) {
  while (!(u->SR & 0x0080)) {
  } // wait until Tx buffer empty
  u->DR = (ch & 0xFF);
}

inline bool USART_has_data(USART u) {
  return (u->SR & 0x0020) != 0;
}


inline void USART_wait_for_data(USART u) {
  while (!USART_has_data(u)) {}
}

inline char USART_read(USART u) {
  while (!(u->SR & 0x0020)) {
  } // Wait until there is data to read
  return u->DR & 0xFF;
}

inline void USART_flush(USART u) {
	while (USART_has_data(u)) {
		// I think this needs to be volatile or else it will just
		// not compile it with optimizations turned on.
		volatile int _discard = u->DR;
	}
}

int USART_write_string(USART u, const char *str) {
  // Write each character of the string until a null byte is encountered.
	int i = 0;
  while (str[i] != '\x00') {
    USART_write(u, str[i]);
		i++;
  }
	return i;
}

int USART_write_string_len(USART u, const char* str, size_t size) {
	for (int i = 0; i < size; i++) {
		if (!str[i]) return i;
		USART_write(u, str[i]);
	}
	return size;
}

int USART_read_string(USART u, char *buffer, int len) {
  // Iterate over the buffer, setting the character at i
  // Stop one short to make room for a null terminator
  for (int i = 0; i < len - 1; i++) {
    char data = USART_read(u);
    // If we get a carriage read or newline, write a newline and then return.
    if (data == '\r' || data == '\n') {
      USART_write(u, '\n');
      // MUST have a null terminator
      buffer[i] = '\n';
      return len; // We did not exceed `len`
    }
    // Else if we got a backspace, print a backspace and then continue
    else if (data == '\b') {
      USART_write(u, '\b');
      i--; // This char doesn't count, so don't increment.
      continue;
    } else {
      buffer[i] = data;
    }
  }
  buffer[len - 1] = '\x00';
  return len;  // We exceeded `len`
}

ssize_t USART_Cookie_read(void* cookie, char* buf, size_t size) {
	// DBG("Reading buffer...\n");
	return USART_read_string((USART)cookie, buf, size);
}

ssize_t USART_Cookie_write(void* cookie, const char* buf, size_t size) {
	// DBG("Writing buffer...\n");
	return USART_write_string_len((USART)cookie, buf, size);
}

int USART_Cookie_seek(void* cookie, off_t* offset, int whence) {
	// DBG("Seeking buffer...\n");
	return 0;
}

int USART_Cookie_close(void* cookie) {
	// DBG("Closing...\n");
	return 0;
}

FILE* USART_fopen(USART u) {
	// Open a USART device as a file with write permissions, returning a file handle
	// that allows the USART device to be read from and written to. Only works with GCC
	// due to requiring the cookies extension for custom streams. Thankfully, PlatformIO
	// uses GCC.

	// Create the cookie functions
	cookie_io_functions_t c = {
		.read = USART_Cookie_read,
		.write = USART_Cookie_write,
		.seek = USART_Cookie_seek,
		.close = USART_Cookie_close
	};
	// Return the stream
	return fopencookie(u, "w+", c);
}