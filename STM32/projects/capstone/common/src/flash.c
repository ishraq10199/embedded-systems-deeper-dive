#include "flash.h"
#include "stm32f411xe.h"
#include "uart.h"
#include <stdio.h>

/***
 * @brief Check if flash is busy (read/write is in progress)
 * @return True if flash is busy, false if not
 */
inline bool flash_busy(void) { return !!(FLASH->SR & FLASH_BUSY); }

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
  /* Here we use DSB to wait before the erasure in case Flash is being accessed
   */
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
 * @brief Initializes the flash for n-bit writes. Assumes flash is unlocked for
 * writes.
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
 * @param address The 32-bit target address. Assumes its a valid address in
 * flash, and that the write was initialized beforehand.
 *
 * @returns The status register bit field with only the relevant error bits set,
 *          if there was any write error, otherwise zero.
 */
int flash_program_word(uint32_t address, uint32_t data) {

  /* The actual write operation */
  *(volatile uint32_t *)address = data;

  /* Data Synchronization Barrier */
  /* This ensures that all memory accesses before it have fully completed */
  /* Here we use DSB to wait before the erasure in case Flash is being accessed
   */
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
    write_err = flash_program_word((uint32_t)destCurr++, *srcStart++);
  }

  if (write_err) {
    handle_write_errors(write_err);
  }

  /* End flash programming */
  flash_program_end();
}

/***
 * @brief Handle write errors by only logging out to the UART.
 * @param flash_write_err The FLASH->SR register value, with only the write
 * error bits unmasked.
 *
 */
void handle_write_errors(int flash_write_err) {
  if (!flash_write_err)
    return;

  if (!uart_tx_initialized()) {
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

/***
 * @brief Compare words sequentially between source and target.
 *
 * @param target A valid 32-bit flash address where the word comparison starts.
 * @param src Source of the data, with which to compare.
 * @param len Length of the data, i.e. how many words to compare.
 *
 * @returns True if there was no difference, false otherwise.
 *
 */
bool validate_sequential_word_write(uint32_t target, uint32_t src,
                                    uint32_t len) {
  uint32_t *wordA = (uint32_t *)target;
  uint32_t *wordB = (uint32_t *)src;

  while (len--) {
    if (*wordA++ ^ *wordB++) {
      return false;
    }
  }

  return true;
}