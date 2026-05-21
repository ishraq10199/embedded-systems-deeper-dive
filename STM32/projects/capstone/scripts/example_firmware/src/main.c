#include "stm32f4xx.h"

#define GPIOCEN (1U << 2)
#define PIN13_SET (1U << 13)
#define PIN13_RESET (1U << 29)
#define LED_PIN_SET PIN13_SET
#define LED_PIN_RESET PIN13_RESET
#define TOGGLE_DELAY_OPS (1000000) // Not in ms or seconds, just arbitrary
static void set_output(void) {
  RCC->AHB1ENR |= GPIOCEN;
  GPIOC->MODER |= (1U << 26);
  GPIOC->MODER &= ~(1U << 27);
}

int main(void) {

  set_output();

  for (;;) {
    GPIOC->BSRR = LED_PIN_SET;
    /**
     * Using `volatile` in these loops because the compile optimization
     * will remove these loops entirely otherwise. The flag -Os optimizes
     * for size, which is commonly done in the release build
     */

    for (volatile int i = 0; i < TOGGLE_DELAY_OPS; i++) {
    }
    GPIOC->BSRR = LED_PIN_RESET;
    for (volatile int i = 0; i < TOGGLE_DELAY_OPS; i++) {
    }
  }
}
