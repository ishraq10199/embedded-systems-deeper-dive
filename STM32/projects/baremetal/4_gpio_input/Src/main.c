#include "stm32f4xx.h"

#define GPIOAEN				(1U << 0)
#define GPIOCEN				(1U << 2)

#define PIN13				(1U << 13)
#define BTN_PIN				PIN13

#define PIN5_SET			(1U << 5)
#define PIN5_RESET			(1U << 21)

#define LED1_PIN_SET		PIN5_SET
#define LED1_PIN_RESET		PIN5_RESET


int main(void) {

	// Enable clock access for GPIOA and GPIOC

	RCC->AHB1ENR |= GPIOAEN;
	RCC->AHB1ENR |= GPIOCEN;

	// Set PA5 as output pin
	GPIOA->MODER |= (1U << 10);
	GPIOA->MODER &= ~(1U << 11);

	// Set PC13 as input pin
	GPIOC->MODER &= ~(1U << 26);
	GPIOC->MODER &= ~(1U << 27);


	for (;;) {
		// Check if BTN is pressed (its active low, i.e. its set by default)
		if (GPIOC->IDR & BTN_PIN) {
			GPIOA->BSRR = LED1_PIN_RESET;
		} else {
			GPIOA->BSRR = LED1_PIN_SET;
		}

	}
}
