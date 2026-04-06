#include "stm32f4xx.h"

#include "systick.h"

#define SYSTICK_LOAD_VAL				(16000)
#define ONE_SECOND_LOAD_VAL				(16000000)
#define CTRL_ENABLE						(1U << 0)
#define CTRL_CLKSRC						(1U << 2)
#define CTRL_COUNTFLAG					(1U << 16)
#define SYSTICK_TICKINT					(1U << 1)

/*
 * Mindset:
 *
 * If a system has a clock of 10K Hz, i.e. it 10K clocks/second,
 * It means that it has 10 clocks/millisecond.
 *
 * We put 10 in the LOAD register, so that when the counter counts
 * down from 10 to 0, the CTRL_COUNTFLAG is set on each reset. i.e.
 *
 * 10...9...8...7...6...5...4...3...2...[1...0]
 * COUNTFLAG set when [1...0] transition happens, and is reset when CTRL is read
 *
 * In this case, it means that 10 clocks passed, so 1 ms has elapsed.
 *
 * To make an `n` millisecond delay, we put the above process in a loop for `n` times.
 */


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


void systick_1hz_interrupt(void) {
	/** Configure the timer **/
	/* Reload with number of clocks in 1 second */
	SysTick->LOAD = ONE_SECOND_LOAD_VAL - 1;

	/* Clear the current value in the timer register */
	SysTick->VAL = 0;

	/* Enable systick and select the internal clock source */
	SysTick->CTRL = (CTRL_ENABLE | CTRL_CLKSRC);

	/* Enable systick interrupt (exception) */
	SysTick->CTRL |= SYSTICK_TICKINT;

	/* NVIC is not involved here, as systick is a core part of the processor */
}
