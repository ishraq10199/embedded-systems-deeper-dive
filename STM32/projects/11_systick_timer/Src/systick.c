#include "stm32f4xx.h"

#define SYSTICK_LOAD_VAL				(16000)
#define CTRL_ENABLE						(1U << 0)
#define CTRL_CLKSRC						(1U << 2)
#define CTRL_COUNTFLAG					(1U << 16)

void systickDelayMs(int delay) {
	/** Configure the timer **/
	/* Reload with number of clocks per ms */
	SysTick->LOAD = SYSTICK_LOAD_VAL;

	/* Clear the current value in the timer register */
	SysTick->VAL = 0;

	/* Enable systick and select the internal clock source */
	SysTick->CTRL = (CTRL_ENABLE | CTRL_CLKSRC);

	for (int i = 0; i < delay; i++) {
		/* Wait until the COUNTFLAG is set */
		while ((SysTick->CTRL & CTRL_COUNTFLAG) == 0) {}
	}

	/* Disable systick and clear the register */
	SysTick->CTRL = 0;
}
