/*
 * Using simple bare-metal addresses to blink LED1 (PA5)
 */

#define PERIPH_BASE 			(0x40000000UL)
#define PERIPH_AHB1_OFFSET		(0x00020000UL)
#define AHB1_BASE				(PERIPH_BASE + PERIPH_AHB1_OFFSET)

#define AHB1_GPIOA_OFFSET		(0x0UL)
#define GPIOA_BASE				(AHB1_BASE + AHB1_GPIOA_OFFSET)

#define AHB1_RCC_OFFSET			(0x00003800UL)
#define RCC_BASE				(AHB1_BASE + AHB1_RCC_OFFSET)
#define RCC_AHB1ENR_OFFSET		(0x30UL)
#define AHB1ENR_RCC				(*(volatile unsigned int *)(RCC_BASE + RCC_AHB1ENR_OFFSET))

#define MODER_OFFSET			(0x0UL)
#define GPIOA_MODE_REG			(*(volatile unsigned int *)(GPIOA_BASE + MODER_OFFSET))

// For setting RCC Clock Enable Register
#define GPIOAEN					(1UL << 0)

#define GPIO_ODR_OFFSET			(0x14UL)
#define GPIOA_ODR				(*(volatile unsigned int *)(GPIOA_BASE + GPIO_ODR_OFFSET))
#define PIN5					(1UL << 5)
#define LED1_PIN				PIN5

int main(void) {

	// 1. Enable clock access to GPIOA
	AHB1ENR_RCC |= GPIOAEN;

	// 2. Set PA5 as output pin
	GPIOA_MODE_REG |= (1UL << 10);
	GPIOA_MODE_REG &= ~(1UL << 11);

	// 3. Toggle PA5 in loop, with a delay
	for(;;) {
		GPIOA_ODR ^= LED1_PIN;
		for (unsigned int i = 0; i < 1000000; i++) {}
	}
}
