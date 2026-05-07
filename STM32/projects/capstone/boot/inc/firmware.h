#ifndef FIRMWARE_H
#define FIRMWARE_H

#include "memory_map.h" // IWYU pragma: keep
#include <inttypes.h>

#define FW_METADATA_SIZE (sizeof(FirmwareInfo_t))
#define FW_METADATA_OFFSET_BYTES ((uint32_t)&__approm_size__ - FW_METADATA_SIZE)
#define FW_METADATA_OFFSET_WORDS (FW_METADATA_OFFSET_BYTES / sizeof(uint32_t))
#define FW_METADATA_ADDR                                                       \
  ((uint32_t)&__approm_start__ + FW_METADATA_OFFSET_WORDS)

typedef struct FirmwareInfo_t FirmwareInfo_t;

void test(void);
void print_firmware_info(void);

#endif /* FIRMWARE_H */