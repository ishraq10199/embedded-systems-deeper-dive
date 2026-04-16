#include <stdio.h>
#include <stdint.h>
#include "uart.h"
#include <string.h>
#include "stm32f4xx.h"
#include "faulthandlers.h"

#define GPIOAEN				(1U << 0)
#define GPIOA_5				(1U << 5)
#define LED_PIN				GPIOA_5

#define DIV_0_TRP			(1U << 4)
#define UNALIGNED_TRP	(1U << 3)


int main(void) {

	uart2_tx_init();

	/* Unaligned memory access:
	 *
	 *
	 * [XXXXXXXX] <- each X == 1 byte of data in a buffer
	 * ^
	 * ptr => dereference 4 bytes => OK (ptr is of type uint32_t*)
	 *
	 * Cortex M architecture requires that the value of ptr1 be a multiple of 4
	 *
	 * So, if we do this:
	 *
	 * [XXXXXXXX] <- each X == 1 byte of data in a buffer
	 *   ^
	 *   ptr (= ptr + 1) => dereference 4 bytes => NOT OK
	 *
	 * Here, ptr will NOT be a multiple of 4
	 * It will thus be classified as an unaligned memory access
	 *
	 */

	/* We need to enable faulting on an unaligned memory access*/
	SCB->CCR |= UNALIGNED_TRP;

	uint8_t buf[8];
	uint32_t *ptr;
	uint32_t value;


	for (;;) {

		for (int i = 0; i <= 4; i++) {
			ptr = (uint32_t *)(buf + i);

			printf("ptr is currently at buf + %d\t", (int)((uint32_t)ptr - (uint32_t)buf));

			if (i % 4) {
				printf("\r\n");

			} else {
				printf("OK\r\n");

			}

			/* The following causes the hardfault when i != 0 */
			value = *ptr;

		}

		printf("Putting ptr at buf and restarting...\r\n\r\n");
	}

}

