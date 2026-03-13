#include "stm32f4xx.h"

#define SYS_FREQ			16000000
#define UART_BAUDRATE		115200

#define APB1CLK				SYS_FREQ

#define GPIOAEN				(1U << 0)
#define USART2EN			(1U << 17)

#define CR1_TE				(1U << 3)
#define CR1_UE				(1U << 13)

#define SR_TXE				(1U << 7)

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);

void uart2_write(int ch);
void uart2_tx_init(void);

int __io_putchar(int ch) {
	uart2_write(ch);
	return ch;
}

int main(void) {

	uart2_tx_init();

	for (;;) {
		printf("Hello world... \r\n");
		for (int i = 0; i < 1000000; i++) {}
	}

}

void uart2_write(int ch) {
	/* Ensure Transmit Data Register (TDR) is empty (check if SR_TXE is set) */
	while (!(USART2->SR & SR_TXE)) {}

	/* Once it is, write to TDR */
	USART2->DR = (ch & 0xFF);

	/* Reason for using 0xFF:
	 *
	 *  If `ch` is a 32-bit int, `FFFFFF41`, AND-ing with `0xFF` "cleans" the upper bits
	 *  and makes it:			 `00000041` after AND-ing
	 */
}

void uart2_tx_init(void) {
	/*************** Configure USART GPIO Pin ***************/
	/* Enable clock access to GPIOA (for PA2) */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set PA2 mode to alternate function mode */
	GPIOA->MODER &= ~(1U << 4);
	GPIOA->MODER |= (1U << 5);

	/* Set PA2 alternate function type to USART_TX */
	GPIOA->AFR[0] |= (1U << 8);
	GPIOA->AFR[0] |= (1U << 9);
	GPIOA->AFR[0] |= (1U << 10);
	GPIOA->AFR[0] &= ~(1U << 11);

	/*************** Configure USART Module ***************/
	/* Enable clock access to USART2 */
	RCC->APB1ENR |= USART2EN;

	/* Configure baudrate */
	uart_set_baudrate(USART2, APB1CLK, UART_BAUDRATE);

	/* Configure transfer direction */
	USART2->CR1 = CR1_TE;

	/* Enable UART module */
	USART2->CR1 |= CR1_UE;
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate) {
	USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate) {
	return ((PeriphClk + (BaudRate / 2U)) / BaudRate) ;
}
