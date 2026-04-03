#ifndef UART_H_
#define UART_H_

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>

#define SR_TXE		(1U << 7)
#define SR_RXNE		(1U << 5)
#define DMA1_TCIF6	(1U << 21)

void uart2_tx_init(void);
void uart2_tx_rx_init(void);
char uart2_read(void);
void dma1_stream6_init(uint32_t src, uint32_t dest, uint32_t len);
void uart2_rx_interrupt_init(void);

#endif /* UART_H_ */
