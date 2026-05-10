#include "uart.h"
#include "stm32f411xe.h"
#include "tim.h"
#include <stdint.h>
#include <stdio.h>

/**
 ** **************************************************************
 ** ********************* UART CODE - START **********************
 ** **************************************************************
 */

#define SYS_FREQ 16000000
#define UART_BAUDRATE 4800

#define APB1CLK SYS_FREQ

#define GPIOAEN (1U << 0)
#define USART2EN (1U << 17)

#define CR1_TE (1U << 3)
#define CR1_RE (1U << 2)
#define CR1_UE (1U << 13)

#define SR_RXNE (1U << 5)
#define SR_TC (1U << 6)
#define SR_TXE (1U << 7)
#define SR_ORE (1U << 3)

#define PA2_SET (1U << 2)
#define PA3_SET (1U << 3)

#define HEX_A_OFFSET ('A' - 10)

static volatile bool uart2_tx_initialized = false;
static volatile bool uart2_tx_rx_initialized = false;
static volatile bool uart2_rx_echo = true;

char uart2_read(void);

bool uart_tx_initialized(void) {
  return uart2_tx_initialized || uart2_tx_rx_initialized;
}

bool uart_tx_rx_initialized(void) { return uart2_tx_rx_initialized; }

bool uart_rx_initialized(void) { return uart2_tx_rx_initialized; }

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk,
                              uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);

void uart2_write(int ch) {
  while (!(USART2->SR & SR_TXE)) {
  }
  USART2->DR = (ch & 0xFF);
}

void uart_send_byte(uint8_t ch) { uart2_write(ch); }

int __io_putchar(int ch) {
  uart2_write(ch);
  return ch;
}

int __io_getchar(void) {
  volatile uint8_t ch = 0;
  volatile uint32_t temp;

  /* Check if overrun error has occurred, clear if so */
  if (USART2->SR & SR_ORE) {
    /* Clearing the ORE status is done by a software sequence */
    /* First a read from SR  */
    temp = USART2->SR;
    /* Then a read from DR */
    temp = USART2->DR;
  }

  /* Prevent the lint error for unused variable */
  (void)temp;

  ch = uart2_read();
  if (!uart2_rx_echo)
    return ch;

  /* Echo the character */
  if (ch == '\r') {
    ch = '\n';
    uart2_write('\r');
    uart2_write('\n');
  } else {
    uart2_write(ch);
  }

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

void uart_tx_init(void) {
  if (uart2_tx_initialized || uart2_tx_rx_initialized)
    return;

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

  uart2_tx_initialized = true;
  uart2_tx_rx_initialized = false; // since rx not initialized
}

char uart2_read(void) {
  /* Ensure Data Register is NOT empty (check if SR_RXNE is set) */
  while (!(USART2->SR & SR_RXNE)) {
  }

  /* Once it is, return the value in the data register*/
  return USART2->DR & 0xFF;
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk,
                              uint32_t BaudRate) {
  USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate) {
  return ((PeriphClk + (BaudRate / 2U)) / BaudRate);
}

void uart_tx_rx_init(void) {
  if (uart2_tx_rx_initialized)
    return;
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

  setvbuf(stdin, NULL, _IONBF, 0);

  uart2_tx_rx_initialized = true;
  uart2_tx_initialized = true;
}

void uart_tx_deinit(void) {
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
  GPIOA->BSRR |= PA2_SET;

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

  uart2_tx_initialized = false;
  uart2_tx_rx_initialized = false;
}

void uart_tx_rx_deinit(void) {
  /* Wait for the shift register to finish clocking out the last byte */
  while (!(USART2->SR & SR_TC)) {
  }

  /* Pre-set the ODR values to HIGH */
  GPIOA->BSRR |= PA2_SET;
  GPIOA->BSRR |= PA3_SET;

  /* Change the MODER to output, so that manual control can be established */
  /* Do it for PA2 (tx) */
  GPIOA->MODER |= (1U << 4);
  GPIOA->MODER &= ~(1U << 5);
  /* And also for PA3 (rx) */
  GPIOA->MODER |= (1U << 6);
  GPIOA->MODER &= ~(1U << 7);

  /* De-initialize the UART module, i.e. do the reverse of init */
  USART2->CR1 = 0;
  USART2->BRR = 0;
  /* Rest AFR for PA2 (tx) */
  GPIOA->AFR[0] &= ~(1U << 8);
  GPIOA->AFR[0] &= ~(1U << 9);
  GPIOA->AFR[0] &= ~(1U << 10);
  GPIOA->AFR[0] &= ~(1U << 11);
  /* Rest AFR for PA3 (rx) */
  GPIOA->AFR[0] &= ~(1U << 12);
  GPIOA->AFR[0] &= ~(1U << 13);
  GPIOA->AFR[0] &= ~(1U << 14);
  GPIOA->AFR[0] &= ~(1U << 15);

  /* Disable clock access to the UART module and GPIOA */
  RCC->APB1ENR &= ~(USART2EN);
  RCC->AHB1ENR &= ~GPIOAEN;

  uart2_tx_initialized = false;
  uart2_tx_rx_initialized = false;
}

/**
 ** ***********************************************************
 ** ********************** UART CODE - END ********************
 ** ***********************************************************
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

void uart_rx_enable_echo(void) { uart2_rx_echo = true; }
void uart_rx_disable_echo(void) { uart2_rx_echo = false; }
int uart_read_byte(uint8_t *dest, uint32_t timeoutMs) {
  /* If data is already here, we do not initialize the timer */
  if (USART2->SR & SR_RXNE) {
    *dest = USART2->DR & 0xFF;
    return 0;
  }

  uint32_t start = get_tick_ms();

  while (!(USART2->SR & SR_RXNE)) {
    if ((get_tick_ms() - start) >= timeoutMs) {
      return -1;
    }
  }

  *dest = USART2->DR & 0xFF;
  return 0;
}

int uart_read_word(uint32_t *dest, uint32_t timeoutMs) {
  uint32_t val = 0;
  int ret;
  uint8_t tempByte;
  uint32_t timeoutMsPerByte = timeoutMs / 4;

  for (int i = 0; i < 32; i += 8) {
    ret = uart_read_byte(&tempByte, timeoutMsPerByte);
    if (ret == -1)
      return -1;
    val |= ((uint32_t)tempByte << i);
  }

  *dest = val;
  return 0;
}

int uart_read_word_stream(uint32_t *dest, uint32_t len, uint32_t timeoutMs) {
  int ret;
  uint32_t timeoutPerWord = timeoutMs / 500;
  while (len--) {
    ret = uart_read_word(dest, timeoutPerWord);
    if (ret == -1)
      return -1;
    dest++;
  }
  return 0;
}