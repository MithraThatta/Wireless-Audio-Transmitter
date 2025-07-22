/*
 * uart.h
 *
 *  Created on: Jun 26, 2025
 *      Author: vthat
 */
void uart2_set_baud(uint32_t pclk, uint32_t baud);
int __io_putchar(int ch);
void RXNEIEInterrupt(void);
uint8_t bufferpop(void);
void bufferReset(void);
int EndCheck(void);
#ifndef INC_UART_H_
#define INC_UART_H_



#endif /* INC_UART_H_ */
