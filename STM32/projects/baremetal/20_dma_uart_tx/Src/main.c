#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "uart.h"

#define GPIOAEN			(1U << 0)
#define GPIOA_5			(1U << 5)
#define LED_PIN			GPIOA_5

static void dma_uart_callback(void);

int main(void) {

	/* Enable clock access to GPIOA */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set PA5 as output pin */
	GPIOA->MODER |= (1U << 10);
	GPIOA->MODER &= ~ (1U << 11);

	/* Source buffer - just a char array */
	char message[] = "Hello there! This is from the STM32 memory.\r\n";

	uart2_tx_init();

	/* Will transfer only once */
	dma1_stream6_init(
		(uint32_t) message,
		/* Destination is the address of USART2 Data Register */
		(uint32_t) &USART2->DR,
		strlen(message)
	);

	for (;;) {

	}

}

static void dma_uart_callback(void) {
	GPIOA->ODR |= LED_PIN;
}

void DMA1_Stream6_IRQHandler(void) {
	/* We need to specifically check for transfer complete here */
	if (DMA1->HISR & DMA1_TCIF6) {
		/* Clear the flag by writing a 1 to the HIFCR */
		DMA1->HIFCR |= DMA1_TCIF6;

		/* Do something */
		dma_uart_callback();
	}
}
