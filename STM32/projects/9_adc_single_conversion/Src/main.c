#include <stdio.h>
#include "uart.h"
#include "adc.h"

uint32_t value;

int main(void) {
	uart2_tx_init();
	pa1_adc_init();

	for (;;) {
		adc_start_conversion();
		value = adc_read();
		printf("Value: %d\r\n", (int)value);
	}
}


