#include "spi.h"

/* Pins for SPI1
 *
 * SPI1_NSS		=> PA4 	// Hardware NSS (not using this)
 * SPI1_SCK		=> PA5	// SCL
 * SPI1_MISO	=> PA6 	// SDO
 * SPI1_MOSI	=> PA7	// SDA
 *
 * NSS (GPIO)	=> PA9	// Software NSS
 *
 * Alternate function: AF05 (0101) for SCL, MISO, MOSI
 *
 * PA9 is to be set to output mode, so we can select the slave
 *
 */

#define SPI1EN			(1U << 12)
#define GPIOAEN			(1U << 0)
#define SPI1_CPHA		(1U << 0)
#define SPI1_CPOL		(1U << 1)
#define SPI1_RXONLY		(1U << 10)
#define SPI1_LSBFIRST	(1U << 7)
#define SPI1_MSTR		(1U << 2)
#define SPI1_DFF		(1U << 11)
#define SPI1_SSI		(1U << 8)
#define SPI1_SSM		(1U << 9)
#define SPI1_SPE		(1U << 6)
#define SPI1_TXE		(1U << 1)
#define SPI1_BUSY		(1U << 7)
#define SPI1_OVR		(1U << 6)
#define SPI1_RXNE		(1U << 0)
#define PA9_SET			(1U << 9)


void spi1_gpio_init(void) {

	/* Enable clock access to GPIOA */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set Alternate function mode for PA5 */
	GPIOA->MODER &= ~(1U << 10);
	GPIOA->MODER |= (1U << 11);

	/* Set Alternate function mode for PA6 */
	GPIOA->MODER &= ~(1U << 12);
	GPIOA->MODER |= (1U << 13);

	/* Set Alternate function mode for PA7 */
	GPIOA->MODER &= ~(1U << 14);
	GPIOA->MODER |= (1U << 15);

	/* Set mode for PA9 as output pin */
	GPIOA->MODER |= (1U << 18);
	GPIOA->MODER &= ~(1U << 19);

	/* Set alternate function type for PA5 (AF05) */
	GPIOA->AFR[0] |= (1U << 20);
	GPIOA->AFR[0] &= ~(1U << 21);
	GPIOA->AFR[0] |= (1U << 22);
	GPIOA->AFR[0] &= ~(1U << 23);

	/* Set alternate function type for PA6 (AF05) */
	GPIOA->AFR[0] |= (1U << 24);
	GPIOA->AFR[0] &= ~(1U << 25);
	GPIOA->AFR[0] |= (1U << 26);
	GPIOA->AFR[0] &= ~(1U << 27);

	/* Set alternate function type for PA7 (AF05) */
	GPIOA->AFR[0] |= (1U << 28);
	GPIOA->AFR[0] &= ~(1U << 29);
	GPIOA->AFR[0] |= (1U << 30);
	GPIOA->AFR[0] &= ~(1U << 31);

	/* De-select the slave by pulling PA9 HIGH */
	GPIOA->ODR |= PA9_SET;
}

void spi1_config(void) {

	/* Enable clock access for SPI1 */
	RCC->APB2ENR |= SPI1EN;

	/* We will use 4MHz for our SPI baud rate */
	/* f_PCLK (peripheral clock frequency) for our board is 16MHz */
	/* So we need to divide it by 4 to get our required baud rate */
	/* Setting a value of 001 to the BR will do just that */
	SPI1->CR1 |= (1U << 3);
	SPI1->CR1 &= ~(1U << 4);
	SPI1->CR1 &= ~(1U << 5);

	/* Set clock polarity (CPOL) to 1 */
	SPI1->CR1 |= SPI1_CPOL;

	/* Set clock phase (CPHA) to 1 */
	SPI1->CR1 |= SPI1_CPHA;

	/* Enable full duplex by disabling RXONLY mode */
	SPI1->CR1 &= ~(SPI1_RXONLY);

	/* Set MSB to be transmitted first */
	SPI1->CR1 &= ~(SPI1_LSBFIRST);

	/* Set STM32 as the master */
	SPI1->CR1 |= SPI1_MSTR;

	/* Set 8-bit data frame format */
	SPI1->CR1 &= ~(SPI1_DFF);

	/* Enable software slave management (so we can use PA9 for NSS) */
	SPI1->CR1 |= SPI1_SSM;

	/* We do not select the slave now (NSS high) - Prerequisite: SSM is 1 */
	SPI1->CR1 |= SPI1_SSI;

	/* We enable the SPI module */
	SPI1->CR1 |= SPI1_SPE;

}

void spi1_transmit(uint8_t *data, uint32_t size) {
	uint32_t i = 0;
	volatile uint8_t temp;

	/* We will increment `i` until we reach `size` */
	while (i < size) {

		/* Wait until the transmit buffer is empty */
		while (!(SPI1->SR & SPI1_TXE)) {}

		/* Write the data */
		SPI1->DR = data[i++];
	}

	/** Housekeeping steps **/
	/* Wait until the transmit buffer is empty */
	while (!(SPI1->SR & SPI1_TXE)) {}

	/* SPI can be busy with a transfer, so we need to wait */
	/* Wait for the SPI to not be busy */
	while (SPI1->SR & SPI1_BUSY) {}

	/* It is possible for data to be received before the previous data is read */
	/* This results in the overrun (OVR) error, and the receive buffer contents */
	/* are not updated with the newly received data from the transmitter device */
	/* Thus, a read operation to the DR returns the previous data. All other */
	/* subsequently transmitted half-words are lost. So we need to clear this flag */

	/* To clear the OVR flag, we need to read DR, then access SR */
	temp = SPI1->DR;
	temp = SPI1->SR;
}

void spi1_receive(uint8_t *data, uint32_t size) {

	while (size) {

		/* SPI is designed for full-duplex sync communication */
		/* For every bit shifted out the Master, a bit is shifted in from the slave */
		/* So to receive data, we must transmit as well */

		/* Send dummy data */
		SPI1->DR = 0;

		/* Wait for new data to be received (RXNE set) */
		while (!(SPI1->SR & SPI1_RXNE)) {}

		/* Store the received data in our buffer */
		*data++ = SPI1->DR;
		size--;

	}
}

void spi1_cs_enable(void) {

	/* We set PA9 to LOW to select the slave (NSS) */
	GPIOA->ODR &= ~(PA9_SET);

}

void spi1_cs_disable(void) {

	/* We set PA9 to HIGH to de-select the slave (NSS) */
	GPIOA->ODR |= PA9_SET;

}

