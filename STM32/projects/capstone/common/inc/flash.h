#ifndef FLASH_H
#define FLASH_H

#include <inttypes.h>
#include <stdbool.h>

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
#define FLASH_WRITE_ERR_MASK                                                   \
  (FLASH_PGSERR | FLASH_PGPERR | FLASH_PGAERR | FLASH_WRPERR)

typedef enum FlashWriteSize {

  FLASH_WRITE_SIZE_BYTE,
  FLASH_WRITE_SIZE_HALFWORD,
  FLASH_WRITE_SIZE_WORD,
  FLASH_WRITE_SIZE_DOUBLEWORD,

} FlashWriteSize;

/* Flash interface functions */

bool flash_busy(void);
void flash_lock(void);
void flash_unlock(void);
void flash_sector_erase(uint8_t sector);
void flash_program_begin(FlashWriteSize sz);
void flash_program_end(void);
int flash_program_word(uint32_t address, uint32_t data);
void write_words_flash(uint32_t dest, uint32_t src, uint32_t size);
void handle_write_errors(int flash_write_err);
void wait_for_pending_flash_operations(void);
bool validate_sequential_word_write(uint32_t target, uint32_t src,
                                    uint32_t len);

#endif /* FLASH_H */