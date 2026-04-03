#include <stdint.h>
#ifndef ADXL345_H_
#define ADXL345_H_

#include "i2c.h"

#define DEVID_R				(0x00)
#define DEVICE_ADDR			(0x53)
#define DATA_FORMAT_R		(0x31)
#define POWER_CTL_R			(0x2D)
#define DATA_START_ADDR		(0x32)

// Range set - Data format register's Bit 1 and Bit 0
#define RANGE_4G			(0x01)

// Power control register
#define POWER_RESET			(0x00)

// Set 0 for standby mode, 1 for measure mode
#define SET_MEASURE_B		(0x08)

// Scale factor for +/- 4g is typically 7.8 mg/LSB (so its 0.0078 g/LSB)
#define SCALE_FACTOR_4G 0.0078;

void adxl_init(void);
void adxl_read_values(uint8_t reg);

#endif /* ADXL345_H_ */
