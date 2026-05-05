#include "uart.h"
#include "stm32f411xe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 ** *********************************************************************************
 ** ******************************* UART CODE - START
 ********************************
 ** *********************************************************************************
 */

#define SYS_FREQ 16000000
#define UART_BAUDRATE 115200

#define APB1CLK SYS_FREQ

#define GPIOAEN (1U << 0)
#define USART2EN (1U << 17)

#define CR1_TE (1U << 3)
#define CR1_RE (1U << 2)
#define CR1_UE (1U << 13)

#define SR_RXNE (1U << 5)
#define SR_TC (1U << 6)
#define SR_TXE (1U << 7)

#define PA2_SET (1U << 2)

#define HEX_A_OFFSET ('A' - 10)

static volatile bool uart2_initialized = false;

bool uart_initialized(void) { return uart2_initialized; }

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk,
                              uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);

void uart2_write(int ch) {
  while (!(USART2->SR & SR_TXE)) {
  }
  USART2->DR = (ch & 0xFF);
}

int __io_putchar(int ch) {
  uart2_write(ch);
  return ch;
}

/* The following was a minimal way to get UART writes out */
/* Can be forgone in favor of `printf`, since we are using newlib now */
/* If we don't need formatting though, this is a better (low overhead) choice */
//
// static void uart2_puts(const char *s) {
//   while (*s)
//     uart2_write(*s++);
// }

static void uart2_put_n(const char *s, int n) {
  while (n--) {
    uart2_write(*s++);
  }
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

  uart2_initialized = true;
}

char uart2_read(void) {
  /* Ensure Data Register is NOT empty (check if SR_RXNE is set) */
  while (!(USART2->SR & SR_RXNE)) {
  }

  /* Once it is, return the value in the data register*/
  return USART2->DR;
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk,
                              uint32_t BaudRate) {
  USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate) {
  return ((PeriphClk + (BaudRate / 2U)) / BaudRate);
}

void uart2_tx_deinit() {
  /* Wait for the shift register to finish clocking out the last byte */
  while (!(USART2->SR & SR_TC)) {
  }

  /* Important: We need to pull the PA2 state to HIGH and change the MODER */
  /* The reason for this is that when we disable USART, and the MODER is still
  ** in AF mode, The pin may momentarily go from HIGH to LOW, sending an
  ** unintended UART start bit. So first we need to set the pin to HIGH
  ** manually, then change the MODER before changing CR1
  */

  /* Pre-set the ODR value to HIGH */
  GPIOA->BSRR = PA2_SET;

  /* Change the MODER to output, so that manual control can be established */
  GPIOA->MODER |= (1U << 4);
  GPIOA->MODER &= ~(1U << 5);

  /* De-initialize the UART module, i.e. do the reverse of init */
  USART2->CR1 = 0;
  USART2->BRR = 0;
  GPIOA->AFR[0] &= ~(1U << 8);
  GPIOA->AFR[0] &= ~(1U << 9);
  GPIOA->AFR[0] &= ~(1U << 10);
  GPIOA->AFR[0] &= ~(1U << 11);

  /* Return PA2 mode to its reset value */
  GPIOA->MODER &= ~(1U << 4);
  GPIOA->MODER &= ~(1U << 5);

  /* Disable clock access to the UART module and GPIOA */
  RCC->APB1ENR &= ~(USART2EN);
  RCC->AHB1ENR &= ~GPIOAEN;

  uart2_initialized = false;
}

/**
 ** *********************************************************************************
 ** ******************************** UART CODE - END
 *********************************
 ** *********************************************************************************
 */

void print_sequential_bytes(uint8_t *src, int length, ByteMode mode) {

  uint8_t ch;

  uint8_t hexLow = 0;
  uint8_t hexHigh = 0;

  switch (mode) {
  case BYTE_MODE_ASCII:
    uart2_put_n((const char *)src, length);
    break;

  case BYTE_MODE_HEX:
    while (length--) {
      ch = *(src);
      hexHigh = (0xF0 & ch) >> 4;
      hexLow = 0x0F & ch;
      uart2_write('0');
      uart2_write('x');
      uart2_write(hexHigh < 10 ? hexHigh + '0' : hexHigh + HEX_A_OFFSET);
      uart2_write(hexLow < 10 ? hexLow + '0' : hexLow + HEX_A_OFFSET);
      uart2_write(' ');

      src++;
    }
    uart2_write('\r');
    uart2_write('\n');
    break;
  }
}

void print_sequential_words(uint32_t *src, int length) {

  uint32_t currword;

  uint8_t hexchar = 0;

  while (length--) {

    currword = *(src);

    uart2_write('0');
    uart2_write('x');

    for (int i = 28; i >= 0; i -= 4) {
      hexchar = (currword >> i) & 0xF;

      uart2_write(hexchar < 10 ? hexchar + '0' : hexchar + HEX_A_OFFSET);
    }

    uart2_write(' ');

    src++;
  }
  uart2_write('\r');
  uart2_write('\n');
}