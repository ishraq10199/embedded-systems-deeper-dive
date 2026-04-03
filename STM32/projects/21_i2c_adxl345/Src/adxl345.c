#include "adxl345.h"

char data;
uint8_t data_vals[6];

void adxl_read_address(uint8_t reg) {
	I2C1_byteRead(DEVICE_ADDR, reg, &data);
}

void adxl_write(uint8_t reg, char value) {
	char data[1];
	data[0] = value;
	I2C1_burstWrite(DEVICE_ADDR, reg, 1, data);
}

void adxl_read_values(uint8_t reg) {
	I2C1_burstRead(DEVICE_ADDR, reg, 6, (char *) data_vals);
}

void adxl_init(void) {
	/* Enable I2C module */
	I2C1_init();

	/* We read the DEVID, which should be 0xE5 */
	adxl_read_address(DEVID_R);

	/* Set the data format range to +/- 4g */
	adxl_write(DATA_FORMAT_R, RANGE_4G);

	/* Reset all bits */
	adxl_write(POWER_CTL_R, POWER_RESET);

	/* Configure the power control measure bit */
	adxl_write(POWER_CTL_R, SET_MEASURE_B);
}
