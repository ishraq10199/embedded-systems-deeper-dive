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
	tim2_output_compare();

	for (;;) {
		// Nothing needed here for the toggle
		// The output data register gets toggled when the timeout occurs
	}
}


