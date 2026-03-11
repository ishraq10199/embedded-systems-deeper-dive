/*
 * Using structs to do the blinking on LED2 (PC9)
 */

#include <stdint.h>

#define PERIPH_BASE 			(0x40000000UL)
#define PERIPH_AHB1_OFFSET		(0x00020000UL)
#define AHB1_BASE				(PERIPH_BASE + PERIPH_AHB1_OFFSET)
#define AHB1_GPIOA_OFFSET		(0x0000UL)
#define AHB1_GPIOC_OFFSET		(0x0800UL)

#define GPIOA_BASE				(AHB1_BASE + AHB1_GPIOA_OFFSET)
#define GPIOC_BASE				(AHB1_BASE + AHB1_GPIOC_OFFSET)

#define AHB1_RCC_OFFSET			(0x00003800UL)
#define RCC_BASE				(AHB1_BASE + AHB1_RCC_OFFSET)

// For setting RCC Clock Enable Regi ster
#define GPIOAEN					(1UL << 0)
#define GPIOCEN					(1UL << 2)

#define PIN5					(1UL << 5)
#define PIN9					(1UL << 9)
#define LED1_PIN				PIN5
#define LED2_PIN				PIN9

typedef struct {

	volatile uint32_t DUMMY[12];
	volatile uint32_t AHB1ENR;

} RCC_TypeDef;

typedef struct {

	volatile uint32_t MODER;
	volatile uint32_t DUMMY[4];
	volatile uint32_t ODR;

} GPIO_TypeDef;

#define RCC		((RCC_TypeDef*) RCC_BASE)
#define GPIOC	((GPIO_TypeDef*) GPIOC_BASE)


int main(void) {

	// 1. Enable clock access to GPIOC
	RCC->AHB1ENR |= GPIOCEN;

	// 2. Set PC9 as output pin
	GPIOC->MODER |= (1UL << 18);
	GPIOC->MODER &= ~(1UL << 19);

	// 3. Toggle PC9 in loop, with a delay
	for(;;) {
		GPIOC->ODR ^= LED2_PIN;
		for (unsigned int i = 0; i < 1000000; i++) {}
	}
}
