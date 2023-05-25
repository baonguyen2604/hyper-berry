#include <stddef.h>
#include <stdint.h>

#ifndef MINI_UART_H
#define MINI_UART_H

void mini_uart_init();
void mini_uart_putc(unsigned char c);
unsigned char mini_uart_getc();
void mini_uart_puts(const char* str);

#endif