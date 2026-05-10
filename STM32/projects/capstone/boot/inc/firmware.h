#ifndef FIRMWARE_H
#define FIRMWARE_H

#include "memory_map.h" // IWYU pragma: keep
#include <inttypes.h>
#include <stdbool.h>

#define FW_METADATA_SIZE (sizeof(FirmwareInfo_t))
#define FW_METADATA_WORDS (FW_METADATA_SIZE / sizeof(uint32_t))
#define FW_METADATA_OFFSET_BYTES ((uint32_t)&__approm_size__ - FW_METADATA_SIZE)
#define FW_METADATA_OFFSET_WORDS (FW_METADATA_OFFSET_BYTES / sizeof(uint32_t))
#define FW_METADATA_ADDR                                                       \
  ((uint32_t)&__approm_start__ + FW_METADATA_OFFSET_BYTES)

/* Communication Bytes */
#define FW_UPDATE_ACK (0xAA)
#define FW_UPDATE_SYNC (0x55)
#define FW_UPDATE_RSND (0x22)
#define FW_UPDATE_REQ (0x11)
#define FW_UPDATE_FIN (0xFF)
#define FW_UPDATE_ERR (0x66)

#define FW_UNDEFINED_VERSION (0xFFFFFFFF)

#define FLASH_APPROM_SECTOR (1U)

typedef struct FirmwareInfo_t {

  uint32_t version;
  uint32_t size;
  uint32_t crc;

} FirmwareInfo_t;

void test(void);
FirmwareInfo_t *get_firmware_info(void);
void print_firmware_info(void);
bool update_firmware_via_uart(void);

#endif /* FIRMWARE_H */