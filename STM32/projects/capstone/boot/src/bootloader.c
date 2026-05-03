#include "memory_map.h"
#include "shared.h"
#include "stm32f411xe.h"
#include "uart.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/**
 ** Load R1 into MSP (msr => Move immediate value to System Register)
 ** Branch to the address in r0 (bx => branch and exchange)
 **/
__attribute__((naked)) static void start_app(uint32_t pc, uint32_t sp) {
  __asm("               \n\
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
  printf("********************************\r\n");
  printf("**** BOOTLOADER v4 SAYS HI! ****\r\n");
  printf("********************************\r\n");
  printf("\r\n");

  printf("[BOOTLOADER] Checking printf formatting: %d %c %s\r\n", 42, 'X',
         "Works!");

  /* We can then do something with the boot_count, e.g. call a function if it
   * exceeds 3 */
  /* Use-cases: Auto resetting 3 times in a row means something may have gone
   * wrong */
  /* In such a case, we may need some graceful way to handle the problem */

  printf("[BOOTLOADER] Copying app firmware to executable ram...\r\n");

  printf("[BOOTLOADER] Copy complete!\r\n");
  printf("\r\n");

  char str[] = "Hello world!\r\n";

  print_sequential_bytes((uint8_t *)str, 14, BYTE_MODE_ASCII);

  /**
   ** START OF FLASH WRITE EXPERIMENT
   */

  FLASH->KEYR = 0x45670123;
  FLASH->KEYR = 0xCDEF89AB;

  while (FLASH->SR & (1U << 16)) {
  }

  volatile uint32_t *fsa = (uint32_t *)&__approm_start__;

  /* FLASH MUST BE ERASED (by sector) BEFORE WRITING TO IT */

  /* Activate Flash programming */
  FLASH->CR |= (1U << 1);

  /* Select sector 1 (approm region) for erasure */
  FLASH->CR |= (1U << 3);
  FLASH->CR &= ~(1U << 4);
  FLASH->CR &= ~(1U << 5);
  FLASH->CR &= ~(1U << 6);

  /* Data Synchronization Barrier */
  /* This ensures that all memory accesses before it have fully completed */
  /* Here we use DSB to wait before the erasure in case Flash is being accessed */
  __DSB();

  /* Start the Flash Erase */
  FLASH->CR |= (1U << 16);

  /* Wait for the Flash memory operation to be over after erase */
  while (FLASH->SR & (1U << 16)) {
  }

  /* Set flash programming size (how many bits in parallel) */
  FLASH->CR |= (1U << 9);
  FLASH->CR &= ~(1U << 8);
  FLASH->CR |= (1U << 0);

  printf(" 0x%08lX \r\n", *(uint32_t *)(fsa));

  volatile uint32_t temp = 0xDEADBEEF;

  *fsa = temp;

  /* Here we wait for the write to complete */
  __DSB();

  /* NOTE: Having a write commit with DSB is needed, as we need consistent values from SR */
  /* If we had not done so, the SR may have given us inconsistent values */
  /* i.e. We don't know what the CPU may do between the write and the SR read */
  /* So, the SR_BUSY flag may be 0, but the write may have not even started */

  /* Wait for the Flash memory operation to be over after write */
  while (FLASH->SR & (1U << 16)) {
  }

  volatile uint32_t flash_sr = FLASH->SR;

  /* Check errors (PGSERR (Bit 7) | PGPERR (Bit 6) | PGAERR (Bit 5) | WRPERR (Bit 4)) */
  if (flash_sr & (0xF << 4)) {
    printf("[BOOTLOADER] ERROR DURING FLASH DATA WRITE!\r\n");

    if (flash_sr & (1U << 7)) {
      printf("\t[BOOTLOADER] CAUSE: PGSERR (Sequence Error)\r\n");
    } else if (flash_sr & (1U << 6)) {
      printf("\t[BOOTLOADER] CAUSE: PGPERR (Parallelism Error)\r\n");
    } else if (flash_sr & (1U << 5)) {
      printf("\t[BOOTLOADER] CAUSE: PGAERR (Alignment Error)\r\n");
    } else if (flash_sr & (1U << 4)) {
      printf("\t[BOOTLOADER] CAUSE: WRPERR (Write Protection Error)\r\n");
    }

    FLASH->SR = 0;
  }

  /* Deactivate Flash programming */
  FLASH->CR &= ~(1U << 0);

  /**
   ** END OF FLASH WRITE EXPERIMENT
   */

  printf(" 0x%08lX \r\n", *(uint32_t *)(fsa));

  print_sequential_bytes((uint8_t *)(0x08000000), 16, BYTE_MODE_HEX);

  uart2_tx_deinit();

  /* We never come here */
  while (1) {
  }
}
