#ifndef UART_H_
#define UART_H_

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>

#define SR_TXE		(1U << 7)
#define SR_RXNE		(1U << 5)

void uart2_tx_init(void);
void uart2_tx_rx_init(void);
char uart2_read(void);
void uart2_rx_interrupt_init(void);

#endif /* UART_H_ */
