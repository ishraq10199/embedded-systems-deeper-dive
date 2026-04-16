#include "stm32f4xx.h"
#include <stdio.h>
#include "systick.h"

#define GPIOAEN				(1U << 0)
#define PIN5				(1U << 5)
#define LED1_PIN			PIN5

int main(void) {

	RCC->AHB1ENR |= GPIOAEN;
	GPIOA->MODER |= (1U << 10);
	GPIOA->MODER &= ~(1U << 11);

	for (;;) {
		systickDelayMs(500);
		GPIOA->ODR ^= LED1_PIN;
	}
}


