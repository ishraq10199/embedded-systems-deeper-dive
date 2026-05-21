#include "tim.h"
#include "stm32f411xe.h"
#include <stdbool.h>
#include <stdint.h>

#define SYS_FREQ (16000000U)

static volatile uint32_t tick_ms = 0;
static volatile bool timer_initialized = false;

void systick_init(void) {
  if (timer_initialized)
    return;

  /* 1 tick per ms - also enables interrupt */
  SysTick_Config(SYS_FREQ / 1000);

  timer_initialized = true;
}

void systick_deinit(void) {
  /* Disable systick */
  SysTick->CTRL = 0;
  SysTick->VAL = 0;
  timer_initialized = false;
}

uint32_t get_tick_ms(void) { return tick_ms; }

/* Override the Systick interrupt handler */
void SysTick_Handler(void) { tick_ms++; }