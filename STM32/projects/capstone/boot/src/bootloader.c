#include "memory_map.h"
#include "shared.h"
#include "stm32f411xe.h"
#include "uart.h"
#include <inttypes.h>
#include <stdbool.h>
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

#define SECTOR_COUNT (8U)
#define FLASH_SNB_OFFSET (3U)
#define FLASH_SNB_MASK (0xF << FLASH_SNB_OFFSET)
#define FLASH_SECTOR_ERASE_EN (1U << 1)
#define FLASH_SECTOR_ERASE_START (1U << 16)
#define FLASH_BUSY (1U << 16)
#define FLASH_PSIZE_OFFSET (8U)
#define FLASH_PSIZE_MASK ((1U << 8) | (1U << 9))
#define FLASH_PG_EN (1U << 0)
#define FLASH_LOCKED (1U << 31)
#define FLASH_UNLOCK_KEY_0 (0x45670123)
#define FLASH_UNLOCK_KEY_1 (0xCDEF89AB)

#define FLASH_PGSERR (1U << 7)
#define FLASH_PGPERR (1U << 6)
#define FLASH_PGAERR (1U << 5)
#define FLASH_WRPERR (1U << 4)
#define FLASH_WRITE_ERR_MASK (FLASH_PGSERR | FLASH_PGPERR | FLASH_PGAERR | FLASH_WRPERR)

typedef enum FlashWriteSize {

  FLASH_WRITE_SIZE_BYTE,
  FLASH_WRITE_SIZE_HALFWORD,
  FLASH_WRITE_SIZE_WORD,
  FLASH_WRITE_SIZE_DOUBLEWORD,

} FlashWriteSize;

/***
 * @brief Check if flash is busy (read/write is in progress)
 * @return True if flash is busy, false if not
 */
inline bool flash_busy(void) {
  return !!(FLASH->SR & FLASH_BUSY);
}

/***
 * @brief Lock the flash to prevent erase/writes
 */
void flash_lock(void) {
  /* Lock flash */
  FLASH->CR |= FLASH_LOCKED;
}

/***
 * @brief Unlock the flash for erase/writes
 */
void flash_unlock(void) {

  /* Check if flash is already unlocked */
  if (!(FLASH->CR & FLASH_LOCKED)) {
    return;
  }

  /* Making these writes sequentially unlocks the Flash */
  FLASH->KEYR = FLASH_UNLOCK_KEY_0;
  FLASH->KEYR = FLASH_UNLOCK_KEY_1;

  while (FLASH->SR & FLASH_BUSY) {
  }
}

/***
 * @brief Erase a flash sector by its index
 * @param sector Ranges from 0 to (SECTOR_COUNT - 1), does nothing otherwise
 *               Directly relates to the 4-bit SNB value in FLASH->CR
 */
void flash_sector_erase(uint8_t sector) {

  /* Activate sector erase */
  FLASH->CR |= FLASH_SECTOR_ERASE_EN;

  if (sector >= SECTOR_COUNT) {
    return;
  }

  /* Select sector 1 (approm region) for erasure */
  FLASH->CR &= ~(FLASH_SNB_MASK);
  FLASH->CR |= (sector << FLASH_SNB_OFFSET);

  /* Data Synchronization Barrier */
  /* This ensures that all memory accesses before it have fully completed */
  /* Here we use DSB to wait before the erasure in case Flash is being accessed */
  __DSB();

  /* Start the Flash Erase */
  FLASH->CR |= FLASH_SECTOR_ERASE_START;

  /* Here we use DSB to wait until the erase is committed */
  __DSB();

  /* Wait for the Flash memory operation to be over after erase */
  while (FLASH->SR & FLASH_BUSY) {
  }

  /* Deactivate sector erase */
  FLASH->CR &= ~(FLASH_SECTOR_ERASE_EN);
}

/***
 * @brief Initializes the flash for n-bit writes. Assumes flash is unlocked for writes.
 *
 * @param sz Enum value relates directly with the two-bit PSIZE value.
 */
void flash_program_begin(FlashWriteSize sz) {

  /* Clear the programming size bits  */
  FLASH->CR &= ~(FLASH_PSIZE_MASK);

  /* Set flash programming size */
  FLASH->CR |= (sz << FLASH_PSIZE_OFFSET);

  /* Enable flash programming */
  FLASH->CR |= FLASH_PG_EN;
}

/***
 * @brief End the word-write. Disables flash programming.
 */
void flash_program_end(void) {
  /* Disable flash programming */
  FLASH->CR &= ~(FLASH_PG_EN);
}

/***
 * @brief Writes a single word (32-bit) to the given address.
 *        Assumes flash is ready for writes.
 *
 * @param address The 32-bit target address. Assumes its a valid address in flash,
 *                and that the write was initialized beforehand.
 *
 * @returns The status register bit field with only the relevant error bits set,
 *          if there was any write error, otherwise zero.
 */
int flash_program_word(uint32_t address, uint32_t data) {

  /* The actual write operation */
  *(volatile uint32_t *)address = data;

  /* Data Synchronization Barrier */
  /* This ensures that all memory accesses before it have fully completed */
  /* Here we use DSB to wait before the erasure in case Flash is being accessed */
  __DSB();

  /* Wait for the Flash memory operation to be over after write */
  while (FLASH->SR & FLASH_BUSY) {
  }

  /* Check write errors */
  /* Check errors  */
  int sr_err = FLASH->SR & FLASH_WRITE_ERR_MASK;

  if (sr_err) {
    /* Clear the error flags (W1C) */
    FLASH->SR |= FLASH_WRITE_ERR_MASK;
    /* We should pass this off to an error handler */
    return sr_err;
  }

  return 0;
}

/***
 * @brief Handle write errors by only logging out to the UART.
 * @param flash_write_err The FLASH->SR register value, with only the write error bits unmasked.
 *
 */
void handle_write_errors(int flash_write_err) {
  if (!flash_write_err)
    return;

  if (!uart_initialized()) {
    return;
  }

  printf("[BOOTLOADER] ERROR DURING FLASH DATA WRITE!\r\n");

  if (flash_write_err & FLASH_PGSERR) {
    printf("[BOOTLOADER] CAUSE: PGSERR (Sequence Error)\r\n");
  } else if (flash_write_err & FLASH_PGPERR) {
    printf("[BOOTLOADER] CAUSE: PGPERR (Parallelism Error)\r\n");
  } else if (flash_write_err & FLASH_PGAERR) {
    printf("[BOOTLOADER] CAUSE: PGAERR (Alignment Error)\r\n");
  } else if (flash_write_err & FLASH_WRPERR) {
    printf("[BOOTLOADER] CAUSE: WRPERR (Write Protection Error)\r\n");
  }
}

/**
 * TODO: REWRITE THIS FUNCTION PROPERLY!!!
 **/

/***
 * @brief Sequentially write data into flash, in words.
 * @param dest Valid flash address
 * @param src Source address
 *
 */
void write_words_flash(uint32_t dest, uint32_t src, uint32_t size) {

  /* Prepare the flash for the writes */
  flash_program_begin(FLASH_WRITE_SIZE_WORD);

  volatile uint32_t *srcStart = (uint32_t *)src;
  volatile uint32_t *destCurr = (uint32_t *)dest;

  int i = 0;
  int write_err = 0;

  /* Do the writes and handle errors */
  while (i++ < size && !write_err) {
    write_err = flash_program_word(
        (uint32_t)destCurr++,
        *srcStart++);
  }

  if (write_err) {
    handle_write_errors(write_err);
  }

  /* End flash programming */
  flash_program_end();
}

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

  /* Unlock the flash */
  flash_unlock();

  /* Erase sector 1 (approm region) */
  flash_sector_erase(1);

  /* Initialize flash write */
  flash_program_begin(FLASH_WRITE_SIZE_WORD);

  /* Use the first word address of the approm region as the test target */
  volatile uint32_t *testTarget = (uint32_t *)&__approm_start__;

  /* Should be 0xFFFFFFFF after sector erasure */
  printf(" 0x%08lX \r\n", *(uint32_t *)(testTarget));

  /* Value we want to write to the test target */
  volatile uint32_t temp = 0xDEADBEEF;
  volatile uint32_t temp2[] = {0xC001CAFE, 0xF00DFACE, 0x8BADF00D, 0xDEADBEEF};

  /* Write to the test target address */
  *testTarget = temp;
  write_words_flash((uint32_t)testTarget + sizeof(uint32_t), (uint32_t)temp2, 4);

  flash_program_end();

  /* Should be 0xDEADBEEF after a successful write */
  printf(" 0x%08lX \r\n", *(uint32_t *)(testTarget));

  /* Should contain the 4 words after a successful write */
  print_sequential_words((uint32_t *)testTarget, 8);

  /**
   ** END OF FLASH WRITE EXPERIMENT
   */

  uart2_tx_deinit();

  /* We never come here */
  while (1) {
  }
}
