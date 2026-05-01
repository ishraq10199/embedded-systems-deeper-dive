#include "memory_map.h"
#include "shared.h"
#include "stm32f411xe.h"
#include <inttypes.h>

/**
 ** *********************************************************************************
 ** ******************************* UART CODE - START *******************************
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

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);

void uart2_write(int ch) {
  while (!(USART2->SR & SR_TXE)) {
  }
  USART2->DR = (ch & 0xFF);
}

// int __io_putchar(int ch) {
//     uart2_write(ch);
//     return ch;
// }

static void uart2_puts(const char *s) {
  while (*s)
    uart2_write(*s++);
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

char uart2_read(void) {
  /* Ensure Data Register is NOT empty (check if SR_RXNE is set) */
  while (!(USART2->SR & SR_RXNE)) {
  }

  /* Once it is, return the value in the data register*/
  return USART2->DR;
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate) {
  USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate) {
  return ((PeriphClk + (BaudRate / 2U)) / BaudRate);
}

static void uart2_tx_deinit() {
  /* Wait for the shift register to finish clocking out the last byte */
  while (!(USART2->SR & SR_TC)) {
  }

  /* De-initialize the UART module, i.e. do the reverse of init */
  USART2->CR1 = 0;
  USART2->BRR = 0;
  GPIOA->AFR[0] &= ~(1U << 8);
  GPIOA->AFR[0] &= ~(1U << 9);
  GPIOA->AFR[0] &= ~(1U << 10);
  GPIOA->AFR[0] &= ~(1U << 11);
  GPIOA->MODER &= ~(1U << 4);
  GPIOA->MODER &= ~(1U << 5);
  RCC->APB1ENR &= ~(USART2EN);
  RCC->AHB1ENR &= ~GPIOAEN;
}

/**
 ** *********************************************************************************
 ** ******************************** UART CODE - END ********************************
 ** *********************************************************************************
 */

/**
 ** Load R1 into MSP (msr => Move immediate value to System Register)
 ** Branch to the address in r0 (bx => branch and exchange)
 **/
__attribute__((naked)) static void start_app(uint32_t pc, uint32_t sp) {
  __asm("             \n\
          msr msp, r1   \n\
          bx r0         \n\
    ");
}

/* Force this variable in our build, to inspect if the data stays */
volatile uint32_t myvar = 0x12345678;
volatile uint8_t boot_count;

int main(void) {

  /* UART message out */
  uart2_tx_init();
  uart2_puts("********************************\r\n");
  uart2_puts("**** BOOTLOADER v1 SAYS HI! ****\r\n");
  uart2_puts("********************************\r\n");
  uart2_puts("\r\n");

  /* As long as the chip has power, this data will persist across resets */
  shared_data_increment_boot_count();
  boot_count = shared_data_get_boot_count();

  /* We can then do something with the boot_count, e.g. call a function if it exceeds 3 */
  /* Use-cases: Auto resetting 3 times in a row means something may have gone wrong */
  /* In such a case, we may need some graceful way to handle the problem */

  /* For now, we can print a message to UART if the boot count exceeds 3 */
  if (boot_count > 3) {
    uart2_puts("[WARNING] System has rebooted 3 times. Something may be wrong!\r\n");
    shared_data_reset_boot_count();
  }

  /* We need to give the app an predictable system state, by de-initializing the UART */
  uart2_tx_deinit();

  /* Get the pointer to where the app is in ROM */
  uint32_t *app_code = (uint32_t *)&__approm_start__;

  /* First 32-bit word contains the stack pointer init address */
  uint32_t app_sp = app_code[0];

  /* Second 32-bit word contains the address to the Reset Handler */
  uint32_t app_start = app_code[1];

  myvar++;

  /* Load the stack pointer to MSP and jump to the app reset handler for execution */
  start_app(app_start, app_sp);

  /* We never come here */
  while (1) {
  }
}