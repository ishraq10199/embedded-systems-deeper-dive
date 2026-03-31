#include "stm32f4xx.h"
#include <stdio.h>
#include "uart.h"
#include "tim.h"

#define GPIOAEN				(1U << 0)
#define PIN5				(1U << 5)
#define LED1_PIN			PIN5

int main(void) {

	RCC->AHB1ENR |= GPIOAEN;
	GPIOA->MODER |= (1U << 10);
	GPIOA->MODER &= ~(1U << 11);

	uart2_tx_init();
	tim2_1hz_init();

	for (;;) {
		while (!(TIM2->SR & SR_UIF)) {}

		/* Clear the UIF */
		TIM2->SR &= ~SR_UIF;

		printf("A second passes.\r\n");

		GPIOA->ODR ^= LED1_PIN;
	}
}


