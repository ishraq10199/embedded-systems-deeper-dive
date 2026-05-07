#ifndef CRC32_H

/* Standard non-reversed polynomial for CRC32 */
#include <inttypes.h>
#include <stdbool.h>

#define POLYNOMIAL (0x04C11DB7)

uint32_t compute_crc32(uint32_t *data, uint32_t len);
bool validate_data_with_crc32(uint32_t *data, uint32_t len);

#define CRC32_H
#endif /* CRC32_H */