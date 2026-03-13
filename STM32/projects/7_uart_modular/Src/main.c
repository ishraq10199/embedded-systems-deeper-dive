#include <stdio.h>
#include "stm32f4xx.h"
#include "uart.h"



int main(void) {

	uart2_tx_init();

	for (;;) {
		printf("Hello world 2... \r\n");
		for (int i = 0; i < 1000000; i++) {}
	}

}


