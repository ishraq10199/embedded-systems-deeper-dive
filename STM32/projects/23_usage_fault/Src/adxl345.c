#include "adxl345.h"

/* These are from ADXL345 documentation - Figure 36, 37, and 38 */
#define MULTI_BYTE_EN			(0x40)
#define READ_OPERATION			(0x80)

char data;
uint8_t data_vals[6];

/* I2C interaction */

void adxl_read_address_i2c(uint8_t reg) {
	I2C1_byteRead(DEVICE_ADDR, reg, &data);
}

void adxl_write_i2c(uint8_t reg, char value) {
	char data[1];
	data[0] = value;
	I2C1_burstWrite(DEVICE_ADDR, reg, 1, data);
}

void adxl_read_values_i2c(uint8_t reg) {
	I2C1_burstRead(DEVICE_ADDR, reg, 6, (char *) data_vals);
}

void adxl_init_i2c(void) {
	/* Enable I2C module */
	I2C1_init();

	/* We read the DEVID, which should be 0xE5 */
	adxl_read_address_i2c(DEVID_R);

	/* Set the data format range to +/- 4g */
	adxl_write_i2c(DATA_FORMAT_R, RANGE_4G);

	/* Reset all bits */
	adxl_write_i2c(POWER_CTL_R, POWER_RESET);

	/* Configure the power control measure bit */
	adxl_write_i2c(POWER_CTL_R, SET_MEASURE_B);
}

/* SPI interaction */

void adxl_read_spi(uint8_t address) {

	/* Enable multi-byte, and read operation */
	address |= MULTI_BYTE_EN | READ_OPERATION;

	/* Pull CS line LOW to enable the slave */
	spi1_cs_enable();

	/* Transmit only the address */
	spi1_transmit(&address, 1);

	/* Read 6 bytes */
	spi1_receive(data_vals, 6);

	/* Pull CS line HIGH to disable slave */
	spi1_cs_disable();

}

void adxl_write_spi(uint8_t address, char value) {

	uint8_t data[2];

	/* Enable multi-byte and place address into buffer */
	data[0] = address | MULTI_BYTE_EN;

	/* Place data into buffer */
	data[1] = value;

	/* Pull CS line LOW to enable the slave */
	spi1_cs_enable();

	/* Transmit data and address */
	spi1_transmit(data, 2);

	/* Pull CS line HIGH to disable the slave */
	spi1_cs_disable();

}

void adxl_init_spi(void) {

	/* Initialize SPI1 GPIO */
	spi1_gpio_init();

	/* Configure SPI1 */
	spi1_config();

	/* Set the data format range to +/- 4g */
	adxl_write_spi(DATA_FORMAT_R, RANGE_4G);

	/* Reset all bits */
	adxl_write_spi(POWER_CTL_R, POWER_RESET);

	/* Configure the power control measure bit */
	adxl_write_spi(POWER_CTL_R, SET_MEASURE_B);
}

