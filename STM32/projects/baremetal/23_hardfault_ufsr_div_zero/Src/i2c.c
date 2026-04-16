#include "stm32f4xx.h"
#include "i2c.h"

/*
 * I2C1 Pins
 *
 * SCL ----> PB8
 * SDA ----> PB9
 */

#define GPIOBEN						(1U << 1)
#define I2C1EN						(1U << 21)
#define OPEN_DRAIN_PIN8				(1U << 8)
#define OPEN_DRAIN_PIN9				(1U << 9)
#define I2C1_CR_SWRST				(1U << 15)
#define I2C1_CR_FREQ				(1U << 4) // 0b 010000 => 16 MHz

#define I2C_100KHZ					80
#define SD_MODE_MAX_RISE_TIME		17

#define I2C1_CR1_PE					(1U << 0)

#define I2C1_SR2_BUSY				(1U << 1)
#define I2C1_CR1_START				(1U << 8)
#define I2C1_SR1_SB					(1U << 0)
#define I2C1_SR1_ADDR				(1U << 1)
#define I2C1_SR1_TXE				(1U << 7)
#define I2C1_CR1_ACK				(1U << 10)
#define I2C1_CR1_STOP				(1U << 9)
#define I2C1_SR1_RXNE				(1U << 6)
#define I2C1_SR1_BTF				(1U << 2)



void I2C1_init(void) {
	/* Enable clock access to GPIOB */
	RCC->AHB1ENR |= GPIOBEN;

	/* Enable alternate function mode for PB8 and PB9 */
	GPIOB->MODER &= ~(1U << 16);
	GPIOB->MODER |= (1U << 17);

	GPIOB->MODER &= ~(1U << 18);
	GPIOB->MODER |= (1U << 19);

	/* Set PB8 and PB9 output type to open drain */
	GPIOB->OTYPER |= OPEN_DRAIN_PIN8;
	GPIOB->OTYPER |= OPEN_DRAIN_PIN9;

	/* Enable pull-up for PB8 and PB9 */
	GPIOB->PUPDR |= (1U << 16);
	GPIOB->PUPDR &= ~(1U << 17);

	GPIOB->PUPDR |= (1U << 18);
	GPIOB->PUPDR &= ~(1U << 19);

	/* Set alternate function mode for PB8 and PB9 (for I2C its AF04) */
	GPIOB->AFR[1] &= ~(1U << 0);
	GPIOB->AFR[1] &= ~(1U << 1);
	GPIOB->AFR[1] |= (1U << 2);
	GPIOB->AFR[1] &= ~(1U << 3);

	GPIOB->AFR[1] &= ~(1U << 4);
	GPIOB->AFR[1] &= ~(1U << 5);
	GPIOB->AFR[1] |= (1U << 6);
	GPIOB->AFR[1] &= ~(1U << 7);

	/* Enable clock access to I2C1 */
	RCC->APB1ENR |= I2C1EN;

	/* Enter Reset mode */
	I2C1->CR1 |= I2C1_CR_SWRST;

	/* Come out of Reset mode */
	I2C1->CR1 &= ~(I2C1_CR_SWRST);

	/* Configure peripheral clock */
	I2C1->CR2 = I2C1_CR_FREQ;

	/* Set I2C to standard mode, 100KHz clock */
	I2C1->CCR = I2C_100KHZ;

	/* Set Rise Time */
	I2C1->TRISE = SD_MODE_MAX_RISE_TIME;

	/* Enable the I2C1 Module */
	I2C1->CR1 |= I2C1_CR1_PE;

}

/*
 * Read a single byte
 *
 * saddr => slave address (to start the transfer)
 * maddr => NOT master address, but actually the memory address
 * 			This is needed to tell the slave which of its registers we want to read
 */
void I2C1_byteRead(char saddr, char maddr, char* data) {

	volatile int temp;

	/* Wait for the I2C to come out of its busy state */
	while (I2C1->SR2 & I2C1_SR2_BUSY) {}

	/* Start the generation (generate the start condition from the master) */
	I2C1->CR1 |= I2C1_CR1_START;

	/* Wait for the start bit to be set */
	while (!(I2C1->SR1 & I2C1_SR1_SB)) {}

	/* Transmit => [7 bit slave address] + [R/W bit]  */
	I2C1->DR = saddr << 1; // make room for the R/W bit, but we don't read now (0)

	/* We need to wait for the address to be matched by the slave */
	while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {}

	/* Reference manual says that the bit is cleared by reading SR1 (done above) */
	/* and then reading SR2 */
	/* So we need to read SR2 now for the address bit to be cleared */
	temp = I2C1->SR2;

	/* TxE flag is set to indicate that DR is empty and is ready to receive the next byte */
	/* We  wait for it to be set */
	while (!(I2C1->SR1 & I2C1_SR1_TXE)) {}

	/* Tell the slave which memory address we want to read the data from */
	I2C1->DR = maddr;

	/* We again wait for DR to be empty */
	while (!(I2C1->SR1 & I2C1_SR1_TXE)) {}

	/*
	 * We told the slave that we want to read data from a specific memory address
	 * Now, we do almost the same thing, but instead of specifying a memory address,
	 * We read the data which comes back from the slave
	 */

	/* Restart the generation */
	I2C1->CR1 |= I2C1_CR1_START;

	/* Wait for the start bit to be set */
	while (!(I2C1->SR1 & I2C1_SR1_SB)) {}

	/* Transmit => [7 bit slave address] + [R/W bit] */
	I2C1->DR = (saddr << 1) | 1; // we need to read now, so we put a 1 as R/W

	/* We need to wait for the address to be matched by the slave */
	while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {}

	/* Disable ACK - we do this to signify that its the last byte we want */
	I2C1->CR1 &= ~(I2C1_CR1_ACK);

	/* We clear the address flag again */
	temp = I2C1->SR2;

	/* We generate a stop condition */
	I2C1->CR1 |= I2C1_CR1_STOP;

	/* Wait for data register to be populated */
	while (!(I2C1->SR1 & I2C1_SR1_RXNE)) {}

	/* Store the data register value (when populated) in our buffer */
	/* We should advance the pointer as we store the byte */
	*data++ = I2C1->DR;
}

/*
 * Read `n` bytes
 *
 * saddr => slave address (to start the transfer)
 * maddr => NOT master address, but actually the memory address
 * 			This is needed to tell the slave which of its registers we want to read
 * n     => number of bytes to read in burst
 */
void I2C1_burstRead(char saddr, char maddr, int n, char* data) {
	volatile int temp;

	/* Wait for the I2C to come out of its busy state */
	while (I2C1->SR2 & I2C1_SR2_BUSY) {}

	/* Start the generation (generate the start condition from the master) */
	I2C1->CR1 |= I2C1_CR1_START;

	/* Wait for the start bit to be set */
	while (!(I2C1->SR1 & I2C1_SR1_SB)) {}

	/* Transmit => [7 bit slave address] + [R/W bit]  */
	I2C1->DR = saddr << 1; // make room for the R/W bit, but we don't read now (0)

	/* We need to wait for the address to be matched by the slave */
	while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {}

	/* Reference manual says that the bit is cleared by reading SR1 (done above) */
	/* and then reading SR2 */
	/* So we need to read SR2 now for the address bit to be cleared */
	temp = I2C1->SR2;

	/* TxE flag is set to indicate that DR is empty and is ready to receive the next byte */
	/* We  wait for it to be set */
	while (!(I2C1->SR1 & I2C1_SR1_TXE)) {}

	/* Tell the slave which memory address we want to read the data from */
	I2C1->DR = maddr;

	/* We again wait for DR to be empty */
	while (!(I2C1->SR1 & I2C1_SR1_TXE)) {}

	/*
	 * We told the slave that we want to read data from a specific memory address
	 * Now, we do almost the same thing, but instead of specifying a memory address,
	 * We read the data which comes back from the slave
	 */

	/* Restart the generation */
	I2C1->CR1 |= I2C1_CR1_START;

	/* Wait for the start bit to be set */
	while (!(I2C1->SR1 & I2C1_SR1_SB)) {}

	/* Transmit => [7 bit slave address] + [R/W bit] */
	I2C1->DR = (saddr << 1) | 1; // we need to read now, so we put a 1 as R/W

	/* We need to wait for the address to be matched by the slave */
	while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {}

	/* Disable ACK - we do this to signify that its the last byte we want */
	I2C1->CR1 &= ~(I2C1_CR1_ACK);

	/* We clear the address flag again */
	temp = I2C1->SR2;

	/*
	 * This is where burst mode differs
	 * We enable ACK as we need to transfer more than 1 byte
	 */

	/* Enable ACK */
	I2C1->CR1 |= I2C1_CR1_ACK;

	while (n > 0U) {

		/* If we are on the last byte, we do some finishing touches */
		if (n == 1U) {
			/* Disable ACK - we do this to signify that its the last byte we want */
			I2C1->CR1 &= ~(I2C1_CR1_ACK);

			/* We generate a stop condition */
			I2C1->CR1 |= I2C1_CR1_STOP;

			/* Wait for data register to be populated */
			while (!(I2C1->SR1 & I2C1_SR1_RXNE)) {}

			/* Store the data register value (when populated) in our buffer */
			/* We should advance the pointer as we store the byte */
			*data++ = I2C1->DR;

		} else {
			/* Wait for data register to be populated */
			while (!(I2C1->SR1 & I2C1_SR1_RXNE)) {}

			/* Put the data in our buffer */
			*data++ = I2C1->DR;

		}

		/* Decrement n and proceed with the next byte read */
		n--;
	}

}

/*
 * Write `n` bytes
 *
 * saddr => slave address (to start the transfer)
 * maddr => NOT master address, but actually the memory address
 * 			This is needed to tell the slave which of its registers we want to write to
 * n     => number of bytes to write in burst
 */
void I2C1_burstWrite(char saddr, char maddr, int n, char* data) {
	volatile int temp;

	/* Wait for the I2C to come out of its busy state */
	while (I2C1->SR2 & I2C1_SR2_BUSY) {}

	/* Start the generation (generate the start condition from the master) */
	I2C1->CR1 |= I2C1_CR1_START;

	/* Wait for the start bit to be set */
	while (!(I2C1->SR1 & I2C1_SR1_SB)) {}

	/* Transmit => [7 bit slave address] + [R/W bit]  */
	I2C1->DR = saddr << 1; // make room for the R/W bit, but we don't read now (0)

	/* We need to wait for the address to be matched by the slave */
	while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {}

	/* We clear the address flag by reading SR1 (above) and then SR2 */
	temp = I2C1->SR2;

	/* TxE flag is set to indicate that DR is empty and is ready to receive the next byte */
	/* We  wait for it to be set */
	while (!(I2C1->SR1 & I2C1_SR1_TXE)) {}

	/* Tell the slave which memory address we want to write the data to */
	I2C1->DR = maddr;

	for (int i = 0; i < n; i++) {
		/* We wait for DR to be empty */
		while (!(I2C1->SR1 & I2C1_SR1_TXE)) {}

		/* Transmit the data */
		I2C1->DR = *data++;
	}

	/* TXE simply indicates that DR is ready for new data */
	/* BTF indicates that the entire byte has been completed, including the ACK phase */

	/* We wait for the transfer to finish */
	while (!(I2C1->SR1 & I2C1_SR1_BTF)) {}


	/* Generate the stop condition once the transfer is finished */
	I2C1->CR1 |= I2C1_CR1_STOP;


}


