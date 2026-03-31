#include "stm32f4xx.h"
#include <stdio.h>
#include "uart.h"
#include "exti.h"


#define EXTI15_PR			(1U << 13)

#define GPIOAEN				(1U << 0)
#define PIN5				(1U << 5)
#define LED1_PIN			PIN5

static void exti_callback(void);

int main(void) {

	uart2_tx_init();
	pc13_exti_init();

	for (;;) {

	}
}

static void exti_callback(void) {
	printf("Button pressed\r\n");
}

void EXTI15_10_IRQHandler(void) {
	/* Check pending register */
	if ((EXTI->PR & EXTI15_PR) != 0) {
		/* Clear the flag */
		/* From the docs: This bit is cleared by programming it to ‘1’. */
		EXTI->PR |= (EXTI15_PR);
		/* Do something */
		exti_callback();
	}
}


