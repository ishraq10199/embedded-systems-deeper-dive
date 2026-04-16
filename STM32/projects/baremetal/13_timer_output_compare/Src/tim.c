#include "stm32f4xx.h"
#include "tim.h"

#define GPIOAEN				(1U << 0)
#define TIM2EN				(1U << 0)
#define CR1_CEN				(1U << 0)
#define CCMR_OC_TOGGLE		((1U << 4) | (1U << 5))
#define CCER_CH1_ENABLE		(1U << 0)
#define AFR_PA5_TIMER		(1U << 20)

// Make a 1Hz timer using TIMER2
void tim2_1hz_init(void) {

	/* Enable clock access to TIM2 */
	RCC->APB1ENR |= TIM2EN;

	/* Set pre-scaler value */
	TIM2->PSC = 1600 - 1; // 16 M / 1 600 = 10 K

	/* Set auto-reload value */
	TIM2->ARR = 10000 - 1; // 10 K (from prev step) / 10 K (from here) = 1

	/* Clear current counter */
	TIM2->CNT = 0;

	/* Enable timer */
	TIM2->CR1 |= CR1_CEN;
}


// Output compare - timer based toggle
void tim2_output_compare(void) {

	/* Enable clock access to PA5 */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set PA5 to Alternate Function Mode */
	GPIOA->MODER &= ~(1U << 10);
	GPIOA->MODER |= (1U << 11);

	/* Set Alternate Function type to AF01 (TIM2 CH1) */
	GPIOA->AFR[0] |= AFR_PA5_TIMER;

	/* Enable clock access to TIM2 */
	RCC->APB1ENR |= TIM2EN;

	/* Set pre-scaler value */
	TIM2->PSC = 1600 - 1; // 16 M / 1 600 = 10 K

	/* Set auto-reload value */
	TIM2->ARR = 10000 - 1; // 10 K (from prev step) / 10 K (from here) = 1

	/* Set output compare toggle mode */
	TIM2->CCMR1 |= CCMR_OC_TOGGLE;

	/* Enable TIM2 CH1 in compare mode */
	TIM2->CCER |= CCER_CH1_ENABLE;

	/* Clear current counter */
	TIM2->CNT = 0;

	/* Enable timer */
	TIM2->CR1 |= CR1_CEN;
}
