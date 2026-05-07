#include "crc32.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

/***
 * @brief Computes the CRC32 value for a single word
 * @param crc Initial CRC value
 * @param word 32-bit data for which the CRC is to be calculated
 * @returns Computed 32-bit CRC value for the single word
 */
uint32_t compute_crc32_block(uint32_t crc, uint32_t word) {
  crc ^= word;
  for (uint8_t i = 0; i < 32; i++) {
    /* Check MSB */
    if (crc & 0x80000000)
      crc = (crc << 1) ^ POLYNOMIAL;
    else
      crc = (crc << 1);
  }
  return crc;
}

/***
 * @brief Computes the CRC32 value for a sequence of words
 * @param data Address for the start of the word sequence
 * @param len Length of the data (number of words)
 * @returns Computed 32-bit CRC value for the given word sequence
 */
uint32_t compute_crc32(uint32_t *data, uint32_t len) {
  uint32_t crc = 0xFFFFFFFF;
  for (uint32_t i = 0; i < len; i++) {
    crc = compute_crc32_block(crc, data[i]);
  }
  return crc;
}

/***
 * @brief Checks if the data (with CRC appended) is valid
 * @param data Address for the start of the word sequence
 * @param len Length of the data + 1 CRC word inclusive (number of words)
 * @returns True if CRC is valid, false otherwise
 */
bool validate_data_with_crc32(uint32_t *data, uint32_t len) {
  uint32_t crc = compute_crc32(data, len);
  /* Non-zero means CRC32 caught some corruption in the data */
  return !crc;
}