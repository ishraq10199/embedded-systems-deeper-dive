#include "exti.h"

#define GPIOCEN			(1U << 2)
#define SYSCFGEN		(1U << 14)
#define SYSCFG_EXTI13	(1U << 5)
#define EXTI13_UNMASK	(1U << 13)
#define EXTI13_FTEN		(1U << 13)

void pc13_exti_init(void) {
	/* Disable global interrupts (optional, but good practice) */
	__disable_irq();

	/* Enable clock access for GPIOC (since we need input) */
	RCC->AHB1ENR |= GPIOCEN;

	/* Select PC13 for input */
	GPIOC->MODER &= ~(1U << 26);
	GPIOC->MODER &= ~(1U << 27);

	/* Enable clock access to SYSCFG */
	RCC->APB2ENR |= SYSCFGEN;

	/* Select PORTC for EXTI13 */
	SYSCFG->EXTICR[3] |= SYSCFG_EXTI13;

	/* Unmask EXTI13 */
	EXTI->IMR |= EXTI13_UNMASK;

	/* Select which edge to trigger (we use falling trigger) */
	EXTI->FTSR |= EXTI13_FTEN;

	/* Enable EXTI13 line in NVIC */
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	/* If we had disabled global interrupts, we re-enable it */
	__enable_irq();
}
