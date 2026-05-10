#include "test.h"
#include "crc32.h"
#include "flash.h"
#include "memory_map.h"
#include "stdint.h"
#include "stdio.h"
#include "uart.h"

/***
 * @brief Tests the following flash operations:
 *        1. Sector erase (only sector 1)
 *        2. Single word write
 *        3. Sequential word writes
 */
void flashWriteTest0(void) {
  /* Unlock the flash */
  flash_unlock();

  /* Erase sector 1 */
  flash_sector_erase(1);

  /* Start the flash programming sequence */
  flash_program_begin(FLASH_WRITE_SIZE_WORD);

  /* Use the first word address of the appromA region as the test target */
  uint32_t *testTarget = (uint32_t *)&__appromA_start__;

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

  /* Erase sector 1 */
  flash_sector_erase(1);

  /* Start the flash programming sequence */
  flash_program_begin(FLASH_WRITE_SIZE_WORD);

  /* Use the first word address of the appromA region as the test target */
  uint32_t *testTarget = (uint32_t *)&__appromA_start__;

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

/***
 * @brief Tests the following CRC operations:
 *        1. Check CRC value computed over 1 word
 *        2. Check CRC value computed over 2 sequential words
 *        3. Check CRC value computed over 4 sequential words
 */
void crc_test0(void) {
  uint32_t data[] = {0x12345678, 0x23456789, 0x3456789A, 0x456789AB};

  uint32_t crc;

  printf("[CRC Test 0] Computing CRC32 for 1 word...\r\n");
  crc = compute_crc32(data, 1);
  printf("[CRC Test 0] Expected: 0x%08" PRIX32 "\r\n", (uint32_t)0xDF8A8A2B);
  printf("[CRC Test 0] Recieved: 0x%08" PRIX32 "\r\n", crc);

  printf("[CRC Test 0] Computing CRC32 for 2 words...\r\n");
  crc = compute_crc32(data, 2);
  printf("[CRC Test 0] Expected: 0x%08" PRIX32 "\r\n", (uint32_t)0x78151F4D);
  printf("[CRC Test 0] Recieved: 0x%08" PRIX32 "\r\n", crc);

  printf("[CRC Test 0] Computing CRC32 for 4 words...\r\n");
  crc = compute_crc32(data, 4);
  printf("[CRC Test 0] Expected: 0x%08" PRIX32 "\r\n", (uint32_t)0xF62CB9EB);
  printf("[CRC Test 0] Recieved: 0x%08" PRIX32 "\r\n", crc);
}

/***
 * @brief Tests the following CRC operations:
 *        1. Check and Validate CRC value computed over 1 word
 *        2. Check and Validate CRC value computed over 2 sequential words
 *        3. Check and Validate CRC value computed over 4 sequential words
 */
void crc_test1(void) {

  bool valid;

  /* Single data word with 1 CRC word appended */
  uint32_t data_with_crc_A[] = {
      0x12345678,
      0xDF8A8A2B, // CRC
  };

  /* 2 data words with 1 CRC word appended */
  uint32_t data_with_crc_B[] = {
      0x12345678, 0x23456789,
      0x78151F4D, // CRC
  };

  /* 4 data words with 1 CRC word appended */
  uint32_t data_with_crc_C[] = {
      0x12345678, 0x23456789, 0x3456789A, 0x456789AB,
      0xF62CB9EB, // CRC
  };

  printf("[CRC Test 1] Validating CRC32 for CRC appended 2 word message... ");
  valid = validate_data_with_crc32(data_with_crc_A, 2);
  if (valid)
    printf("OK!\r\n");
  else
    printf("MISMATCH!\r\n");

  printf("[CRC Test 1] Validating CRC32 for CRC appended 3 word message... ");
  valid = validate_data_with_crc32(data_with_crc_B, 3);
  if (valid)
    printf("OK!\r\n");
  else
    printf("MISMATCH!\r\n");

  printf("[CRC Test 1] Validating CRC32 for CRC appended 5 word message... ");
  valid = validate_data_with_crc32(data_with_crc_C, 5);
  if (valid)
    printf("OK!\r\n");
  else
    printf("MISMATCH!\r\n");
}