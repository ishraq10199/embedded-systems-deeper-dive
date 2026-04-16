#include <stdio.h>
#include "uart.h"
#include "adc.h"

uint32_t value;

int main(void) {
	uart2_tx_init();
	pa1_adc_continuous_init();
	adc_start_continuous_conversion();
	for (;;) {
		value = adc_read();
		printf("Value: %d\r\n", (int)value);
	}
}


