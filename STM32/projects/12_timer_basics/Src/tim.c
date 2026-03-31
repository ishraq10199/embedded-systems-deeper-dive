#include "stm32f4xx.h"
#include "tim.h"

#define TIM2EN			(1U << 0)
#define CR1_CEN			(1U << 0)

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
