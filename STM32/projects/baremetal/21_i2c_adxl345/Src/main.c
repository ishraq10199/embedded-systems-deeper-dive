#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "uart.h"
#include "adxl345.h"

#define GPIOAEN			(1U << 0)
#define GPIOA_5			(1U << 5)
#define LED_PIN			GPIOA_5

int16_t x, y, z;
float xg, yg, zg;

//extern char data;
extern uint8_t data_vals[6];

int main(void) {

	uart2_tx_init();
	printf("Initializing ADXL module via I2C...\r\n");

	adxl_init();
	printf("ADXL module initialized via I2C!\r\n");


	for (;;) {
		printf("Reading values...\r\n");
		adxl_read_values(DATA_START_ADDR);


		/* Raw/un-scaled values need to be arranged first */

		/* X => [8 bits from X1] + [8 bits from X0] */
		x = ((data_vals[1] << 8) | (data_vals[0]));
		/* Y => [8 bits from Y1] + [8 bits from Y0] */
		y = ((data_vals[3] << 8) | (data_vals[2]));
		/* Z => [8 bits from Z1] + [8 bits from Z0] */
		z = ((data_vals[5] << 8) | (data_vals[4]));

		/* We then convert them into actual g values using the scale factor */

//		xg = x * SCALE_FACTOR_4G;
//		yg = y * SCALE_FACTOR_4G;
//		zg = z * SCALE_FACTOR_4G;

		xg = x * 0.0078;
		yg = y * 0.0078;
		zg = z * 0.0078;

//		printf("%.2f %.2f %.2f\r\n", xg, yg, zg);
	}

}
