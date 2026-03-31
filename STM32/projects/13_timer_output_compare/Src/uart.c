#include "uart.h"

#define SYS_FREQ			16000000
#define UART_BAUDRATE		115200

#define APB1CLK				SYS_FREQ

#define GPIOAEN				(1U << 0)
#define USART2EN			(1U << 17)

#define CR1_TE				(1U << 3)
#define CR1_RE				(1U << 2)
#define CR1_UE				(1U << 13)

#define SR_TXE				(1U << 7)
#define SR_RXNE				(1U << 5)

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);

void uart2_write(int ch);
void uart2_tx_rx_init(void);


int __io_putchar(int ch) {
	uart2_write(ch);
	return ch;
}


void uart2_write(int ch) {
	/* Ensure Data Register (DR) is empty (check if SR_TXE is set) */
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

void uart2_tx_rx_init(void) {
	/*************** Configure USART GPIO Pin ***************/
	/* Enable clock access to GPIOA (for PA2) */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set PA2 mode to alternate function mode for TX */
	GPIOA->MODER &= ~(1U << 4);
	GPIOA->MODER |= (1U << 5);

	/* Set PA3 mode to alternate function mode for RX */
	GPIOA->MODER &= ~(1U << 6);
	GPIOA->MODER |= (1U << 7);

	/* Set PA2 alternate function type to USART_TX - (AFRL2) := (AF) 07 */
	GPIOA->AFR[0] |= (1U << 8);
	GPIOA->AFR[0] |= (1U << 9);
	GPIOA->AFR[0] |= (1U << 10);
	GPIOA->AFR[0] &= ~(1U << 11);

	/* Set PA3 alternate function type to USART_RX - (AFRL3) := (AF) 07 */
	GPIOA->AFR[0] |= (1U << 12);
	GPIOA->AFR[0] |= (1U << 13);
	GPIOA->AFR[0] |= (1U << 14);
	GPIOA->AFR[0] &= ~(1U << 15);

	/*************** Configure USART Module ***************/
	/* Enable clock access to USART2 */
	RCC->APB1ENR |= USART2EN;

	/* Configure baudrate */
	uart_set_baudrate(USART2, APB1CLK, UART_BAUDRATE);

	/* Configure transfer direction for TX and RX */
	USART2->CR1 = (CR1_TE | CR1_RE);

	/* Enable UART module */
	USART2->CR1 |= CR1_UE;
}

char uart2_read(void) {
	/* Ensure Data Register is NOT empty (check if SR_RXNE is set) */
	while (!(USART2->SR & SR_RXNE)) {}

	/* Once it is, return the value in the data register*/
	return USART2->DR;
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate) {
	USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate) {
	return ((PeriphClk + (BaudRate / 2U)) / BaudRate) ;
}
