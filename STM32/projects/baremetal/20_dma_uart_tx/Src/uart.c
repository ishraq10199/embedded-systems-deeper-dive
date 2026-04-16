#include "uart.h"

#define SYS_FREQ					16000000
#define UART_BAUDRATE				115200

#define APB1CLK						SYS_FREQ

#define GPIOAEN						(1U << 0)
#define USART2EN					(1U << 17)

#define CR1_TE						(1U << 3)
#define CR1_RE						(1U << 2)
#define CR1_UE						(1U << 13)
#define CR1_RXNEIE					(1U << 5)

#define DMA1EN						(1U << 21)
#define DMA1_S6_FLAG_RESET			((1U << 16) | (1U << 18) | (1U << 19) | (1U << 20) | (1U << 21))
#define DMA1_CH4_SEL				(1U << 27)
#define DMA1_MINC					(1U << 10)
#define DMA1_DIR_MEM_TO_PERIPH		(1U << 6)
#define DMA1_TCIE					(1U << 4)
#define DMA1_EN						(1U << 0)
#define USART2_DMAT					(1U << 7)



static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);

void uart2_write(int ch);
void uart2_tx_rx_init(void);


/*
 * From the reference manual, checking the DMA1 request mapping,
 * We get that USART2_TX corresponds to Stream 6 Channel 4
 * DMA1 can do transfers between peripheral and memory
 * We need to set the memory and peripheral (base) addresses
 * Then we need to set the direction of transfer
 *
 * High Interrupt (HISR/HIFCR) -> (streams 4-7)
 * Low Interrupt  (LISR/LIFCR) -> (streams 0-3)
 */
void dma1_stream6_init(uint32_t src, uint32_t dest, uint32_t len) {
	/* Enable clock access to DMA */
	RCC->AHB1ENR |= DMA1EN;

	/* Disable DMA1 Stream6 (needed to write to PAR and M0AR) */
	DMA1_Stream6->CR &= ~(DMA1_EN);

	/* Wait for Stream6 to be disabled */
	while (DMA1_Stream6->CR & DMA1_EN) {}

	/* Clear all interrupt flags of Stream6 */
	DMA1->HIFCR |= DMA1_S6_FLAG_RESET;

	/* Set peripheral buffer (PAR -> Peripheral address register) */
	DMA1_Stream6->PAR = dest;

	/* Set source buffer */
	DMA1_Stream6->M0AR = src;

	/* Set length */
	DMA1_Stream6->NDTR = len;

	/* Select Stream6 Ch4 (only one channel to be selected) */
	DMA1_Stream6->CR = DMA1_CH4_SEL;

	/* Enable memory increment (we don't need peripheral increment here) */
	DMA1_Stream6->CR |= DMA1_MINC;

	/* Configure transfer direction (Memory to Peripheral) */
	DMA1_Stream6->CR |= DMA1_DIR_MEM_TO_PERIPH;

	/* Enable DMA Transfer Complete Interrupt */
	DMA1_Stream6->CR |= DMA1_TCIE;

	/* Enable direct mode and disable FIFO (we can clear the full register) */
	DMA1_Stream6->FCR = 0;

	/* Enable DMA1 Stream6 */
	DMA1_Stream6->CR |= DMA1_EN;

	/* Enable USART2 transmitter for DMA (DMAT in the manual) */
	USART2->CR3 |= USART2_DMAT;

	/* Enable DMA1 Stream6 interrupts in NVIC */
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);

}

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

void uart2_rx_interrupt_init(void) {
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

	/* Enable RXNE interrupt */
	USART2->CR1 |= CR1_RXNEIE;

	/* Enable USART interrupt in NVIC */
	NVIC_EnableIRQ(USART2_IRQn);

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
