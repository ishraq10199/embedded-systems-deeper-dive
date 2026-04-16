#include <stdio.h>
#include "uart.h"
#include "adc.h"

void adc_callback(void);

uint32_t value;

int main(void) {
	uart2_tx_init();
	pa1_adc_interrupt_init();
	adc_start_continuous_conversion();

	for (;;) {

	}
}

void adc_callback(void) {
	value = ADC1->DR;
	printf("Value: %d\r\n", (int)value);
}

void ADC_IRQHandler(void) {
	if (ADC1->SR & SR_EOC) {
		/* Clear the flag */
		ADC1->SR &= ~(SR_EOC);

		/* Do something */
		adc_callback();
	}
}
