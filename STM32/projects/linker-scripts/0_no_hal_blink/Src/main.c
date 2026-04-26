#include "stm32f4xx.h"

#define GPIOAEN				(1U << 0)
#define PIN5_SET			(1U << 5)
#define PIN5_RESET			(1U << 21)
#define LED1_PIN_SET		PIN5_SET
#define LED1_PIN_RESET		PIN5_RESET

static void set_output(void) {
	RCC->AHB1ENR |= GPIOAEN;
	GPIOA->MODER |= (1U << 10);
	GPIOA->MODER &= ~(1U << 11);
}

int main(void) {

	set_output();

	for (;;) {
		GPIOA->BSRR = LED1_PIN_SET;
		for (int i = 0; i < 1000000; i++) {}
		GPIOA->BSRR = LED1_PIN_RESET;
		for (int i = 0; i < 1000000; i++) {}
	}
}
