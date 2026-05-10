#ifndef FIRMWARE_H
#define FIRMWARE_H

#include "memory_map.h" // IWYU pragma: keep
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#define FW_METADATA_SIZE (sizeof(FirmwareInfo_t))
#define FW_METADATA_WORDS (FW_METADATA_SIZE / sizeof(uint32_t))
#define FW_METADATA_ADDR ((uint32_t)&__metarom_start__)

/* Communication Bytes */
#define FW_UPDATE_ACK (0xAA)
#define FW_UPDATE_SYNC (0x55)
#define FW_UPDATE_RSND (0x22)
#define FW_UPDATE_REQ (0x11)
#define FW_UPDATE_FIN (0xFF)
#define FW_UPDATE_ERR (0x66)

#define FW_UNDEFINED_VERSION (0xFFFFFFFF)
#define FW_UNDEFINED_REGION (0xFFFFFFFF)

#define METAROM_SECTOR (7U)

#define FLASH_APPROM_A_SECTOR_START (1U)
#define FLASH_APPROM_A_SECTOR_END (4U)
#define FLASH_APPROM_B_SECTOR_START (5U)
#define FLASH_APPROM_B_SECTOR_END (5U)

typedef struct FirmwareInfo_t {

  uint32_t version;
  uint32_t size;
  uint32_t crc;
  uint32_t region;

} FirmwareInfo_t;

typedef enum AppromRegion {
  APPROM_REGION_UNDEFINED = (uint32_t)FW_UNDEFINED_REGION,
  APPROM_REGION_A = 1U,
  APPROM_REGION_B = 2U,

} AppromRegion;

void test(void);
FirmwareInfo_t *get_firmware_info(void);
void print_firmware_info(void);
bool update_firmware_via_uart(void);

uint32_t *get_current_approm_start(void);

#endif /* FIRMWARE_H */