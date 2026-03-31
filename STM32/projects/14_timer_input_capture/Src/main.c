#include "stm32f4xx.h"
#include <stdio.h>
#include "uart.h"
#include "tim.h"

#define GPIOAEN				(1U << 0)
#define PIN5				(1U << 5)
#define LED1_PIN			PIN5

int timestamp = 0;

/* Hardware: Connect PA5 to PA6 */

int main(void) {

	RCC->AHB1ENR |= GPIOAEN;
	GPIOA->MODER |= (1U << 10);
	GPIOA->MODER &= ~(1U << 11);

	uart2_tx_init();
	tim3_pa6_input_capture();
	tim2_pa5_output_compare();

	for (;;) {
		/* Wait until the edge is captured */
		while (!(TIM3->SR & SR_CC1IF)) {}

		/* Read the value */
		timestamp = TIM3->CCR1;

		printf("Detected rising edge at: %d\r\n", timestamp);
	}
}


