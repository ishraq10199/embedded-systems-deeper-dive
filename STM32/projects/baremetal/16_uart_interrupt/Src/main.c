#include "stm32f4xx.h"
#include <stdio.h>
#include "uart.h"
#include "exti.h"


#define EXTI15_PR			(1U << 13)

#define GPIOAEN				(1U << 0)
#define PIN5				(1U << 5)
#define LED1_PIN			PIN5

static void usart2_callback(void);

char key;

int main(void) {

	/* Enable clock access to GPIOA */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set PA5 as output pin */
	GPIOA->MODER |= (1U << 10);
	GPIOA->MODER &= ~ (1U << 11);

	uart2_tx_init();
	uart2_rx_interrupt_init();

	for (;;) {

	}
}

static void usart2_callback(void) {
	key = USART2->DR;
	if (key == '1') {
		GPIOA->ODR ^= LED1_PIN;
	} else {
		GPIOA->ODR &= ~LED1_PIN;
	}
}

void USART2_IRQHandler(void) {
	/* Check pending register */
	if ((USART2->SR & SR_RXNE) != 0) {
		/* Do something */
		usart2_callback();
	}
}


