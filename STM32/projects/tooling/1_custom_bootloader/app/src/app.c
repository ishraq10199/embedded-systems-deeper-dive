#include "stm32f4xx.h"

#define GPIOAEN (1U << 0)
#define PIN5_SET (1U << 5)
#define PIN5_RESET (1U << 21)
#define LED1_PIN_SET PIN5_SET
#define LED1_PIN_RESET PIN5_RESET
#define TOGGLE_DELAY_OPS (500000) // Not in ms or seconds, just arbitrary
static void set_output(void) {
    RCC->AHB1ENR |= GPIOAEN;
    GPIOA->MODER |= (1U << 10);
    GPIOA->MODER &= ~(1U << 11);
}

int main(void) {

    set_output();

    for (;;) {
        GPIOA->BSRR = LED1_PIN_SET;
        /**
         * Using `volatile` in these loops because the compile optimization
         * will remove these loops entirely otherwise. The flag -Os optimizes
         * for size, which is commonly done in the release build
         */

        for (volatile int i = 0; i < TOGGLE_DELAY_OPS; i++) {
        }
        GPIOA->BSRR = LED1_PIN_RESET;
        for (volatile int i = 0; i < TOGGLE_DELAY_OPS; i++) {
        }
    }
}
