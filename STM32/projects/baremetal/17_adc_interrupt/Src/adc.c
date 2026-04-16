#include <stdint.h>
#include "stm32f4xx.h"
#include <stdint.h>
#include "adc.h"

/* Info:
 * ADC Pin => PA1 (ADC1_1)
 *
 * Tasks:
 * Enable clock access to GPIOA (for PA1)
 * PA1 Mode => Analog (11)
 *
 * ADC Clock Enable => APB2ENR (ADC1EN - Bit 8)
 * ADC_SQR3 - Set first conversion in sequence to be ADC_CH1
 * ADC_SQR1 - Set number of conversions (length) => 1 (state 0x00)
 * SET ADC_CR2 control register ADON => Enable (1)
 *
 * START ADC conversion using ADC_CR2 => Set SWSTART to 1
 *
 * Wait for conversion to complete => ADC_SR - EOC flag
 * Read ADC data
 */

#define GPIOAEN				(1U << 0)
#define ADC1EN				(1U << 8)
#define ADC_CH1				(1U << 0)
#define ADC_SEQ_LEN_1		(0x00)
#define CR2_ADON			(1U << 0)
#define CR2_CONT			(1U << 1)
#define CR2_SWSTART			(1U << 30)

#define SR_EOCIE			(1U << 5)

void pa1_adc_single_init(void) {
	/** Configure the ADC pin and mode **/
	/* Enable clock access to GPIOA */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set pin mode of PA1 (ADC channel 1 pin) to Analog */
	GPIOA->MODER |= (1U << 2);
	GPIOA->MODER |= (1U << 3);

	/** Configure the ADC module **/
	/* Enable ADC Clock */
	RCC->APB2ENR |= ADC1EN;

	/* Set conversion sequence */
	ADC1->SQR3 = ADC_CH1;

	/* Set conversion length */
	ADC1->SQR1 = ADC_SEQ_LEN_1;

	/* Enable the ADC module */
	ADC1->CR2 |= CR2_ADON;

}

void pa1_adc_interrupt_init(void) {
	/** Configure the ADC pin and mode **/
	/* Enable clock access to GPIOA */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set pin mode of PA1 (ADC channel 1 pin) to Analog */
	GPIOA->MODER |= (1U << 2);
	GPIOA->MODER |= (1U << 3);

	/** Configure the ADC module **/
	/* Enable ADC Clock */
	RCC->APB2ENR |= ADC1EN;

	/* Set conversion sequence */
	ADC1->SQR3 = ADC_CH1;

	/* Set conversion length */
	ADC1->SQR1 = ADC_SEQ_LEN_1;

	/* Enable the ADC module */
	ADC1->CR2 |= CR2_ADON;

	/* Enable End of Conversion Interrupt */
	ADC1->CR1 |= SR_EOCIE;

	/* Enable ADC interrupts in NVIC */
	NVIC_EnableIRQ(ADC_IRQn);

	/* Enable the ADC module */
	ADC1->CR2 |= CR2_ADON;

}

void pa1_adc_continuous_init(void) {
	/** Configure the ADC pin and mode **/
	/* Enable clock access to GPIOA */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set pin mode of PA1 (ADC channel 1 pin) to Analog */
	GPIOA->MODER |= (1U << 2);
	GPIOA->MODER |= (1U << 3);

	/** Configure the ADC module **/
	/* Enable ADC Clock */
	RCC->APB2ENR |= ADC1EN;

	/* Set conversion sequence */
	ADC1->SQR3 = ADC_CH1;

	/* Set conversion length */
	ADC1->SQR1 = ADC_SEQ_LEN_1;

	/* Enable the ADC module */
	ADC1->CR2 |= CR2_ADON                                                                                                                                               ;

}

void adc_start_single_conversion(void) {
	/* Start the next conversion */
	ADC1->CR2 |= CR2_SWSTART;
}

void adc_start_continuous_conversion(void) {
	/* Enable continuous conversion */
	ADC1->CR2 |= CR2_CONT;
	/* Start the continuous conversion */
	ADC1->CR2 |= CR2_SWSTART;
}

uint32_t adc_read(void) {
	/* Wait for conversion to end if running */
	while (!(ADC1->SR & SR_EOC)) {}

	/* Read ADC converted value */
	return (ADC1->DR);
}
