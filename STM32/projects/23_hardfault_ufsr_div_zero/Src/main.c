#include <stdio.h>
#include "uart.h"
#include <string.h>
#include "stm32f4xx.h"
#include "faulthandlers.h"

#define GPIOAEN			(1U << 0)
#define GPIOA_5			(1U << 5)
#define LED_PIN			GPIOA_5

#define DIV_0_TRP		(1U << 4)


int main(void) {

	uart2_tx_init();

	/* We need to enable faulting on a div-by-zero */
	SCB->CCR |= DIV_0_TRP;

	int x, y, z = 0;

	x = 5;
	y = 3;

	for (;;) {

		printf("Main loop.\tDividing %d by %d\r\n", x, y);

		z += x / y;

		if (y == 0) {
			/* If we reached here, it means our hardfault handler works as expected :) */
			printf("Gracefully exited the hardfault!\r\n\r\n");
			y = 3;
		} else {
			--y;
		}

	}

}

