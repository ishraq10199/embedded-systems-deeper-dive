// #include <string.h>
// #include "shared.h"
// #include "stm32f411xe.h"
#include "flash.h"
#include "memory_map.h"
#include "uart.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 ** Load R1 into MSP (msr => Move immediate value to System Register)
 ** Branch to the address in r0 (bx => branch and exchange)
 **/
__attribute__((unused))
/** TODO: Remove the above when we use the function! */

__attribute__((naked)) static void
start_app(uint32_t pc, uint32_t sp) {
  __asm("               \n\
          msr msp, r1   \n\
          bx r0         \n\
    ");
}

/***
 * @brief Tests the following flash operations:
 *        1. Sector erase (only sector 1)
 *        2. Single word write
 *        3. Sequential word writes
 */
void flashWriteTest0(void) {
  /* Unlock the flash */
  flash_unlock();

  /* Erase sector 1 (approm region) */
  flash_sector_erase(1);

  /* Start the flash programming sequence */
  flash_program_begin(FLASH_WRITE_SIZE_WORD);

  /* Use the first word address of the approm region as the test target */
  uint32_t *testTarget = (uint32_t *)&__approm_start__;

  /* Should be 0xFFFFFFFF after sector erasure */
  printf("[Flash Write Test 0] After erase: 0x%08" PRIX32 "\r\n",
         *(uint32_t *)(testTarget));

  /* Value we want to write to the test target */
  uint32_t temp = 0xDEADBEEF;
  uint32_t temp2[] = {0xC001CAFE, 0xF00DFACE, 0x8BADF00D, 0x2BAD2BAD};

  /* Write to the test target address */
  *testTarget = temp;
  write_words_flash((uint32_t)testTarget + sizeof(uint32_t), (uint32_t)temp2,
                    4);

  /* End the flash programming sequence */
  flash_program_end();

  /* Should be 0xDEADBEEF after a successful write */
  printf("[Flash Write Test 0] After write: 0x%08" PRIX32 "\r\n",
         *(uint32_t *)(testTarget));

  /* Should be the same 4 words from `temp2` after a successful write */
  printf("[Flash Write Test 0] Expected: \t");
  fflush(stdout);
  print_sequential_words(temp2, 4);
  printf("[Flash Write Test 0] Received:\t");
  fflush(stdout);
  print_sequential_words(testTarget + 1, 4);
}

/***
 * @brief Tests the following flash operations:
 *        1. Write sequential words to flash
 *        2. Validate the write using word-wise XOR
 */
void flashWriteTest1(void) {
  /* Unlock the flash */
  flash_unlock();

  /* Erase sector 1 (approm region) */
  flash_sector_erase(1);

  /* Start the flash programming sequence */
  flash_program_begin(FLASH_WRITE_SIZE_WORD);

  /* Use the first word address of the approm region as the test target */
  uint32_t *testTarget = (uint32_t *)&__approm_start__;

  /* Value we want to write to the test target */
  uint32_t temp[] = {0xC001CAFE, 0xF00DFACE, 0x8BADF00D, 0x2BAD2BAD};

  /* Write sequential words to the test target address */
  write_words_flash((uint32_t)testTarget, (uint32_t)temp, 4);

  /* End the flash programming sequence*/
  flash_program_end();

  /* Emulate a successful write - following should say "OK" */
  bool ok =
      validate_sequential_word_write((uint32_t)testTarget, (uint32_t)temp, 4);
  printf("[Flash Write Test 1] Sequential word write validation status: %s\r\n",
         ok ? "OK" : "MISMATCH");

  /* Introduce a mismatch into the src data */
  temp[2] = 0x0EEAEE0E;

  /* Emulate an "unsuccessful" write - following should say "MISMATCH" */
  ok = validate_sequential_word_write((uint32_t)testTarget, (uint32_t)temp, 4);
  printf("[Flash Write Test 1] Sequential word write validation status: %s\r\n",
         ok ? "OK" : "MISMATCH");
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

  /**
   ** START OF FLASH WRITE EXPERIMENT
   */
  flashWriteTest0();
  flashWriteTest1();
  /**
   ** END OF FLASH WRITE EXPERIMENT
   */

  uart2_tx_deinit();

  /* We never come here */
  while (1) {
  }
}
