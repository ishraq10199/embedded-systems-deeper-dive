#include "firmware.h"
#include "crc32.h"
#include "flash.h"
#include "memory_map.h"
#include "stm32f4xx.h"
#include "tim.h"
#include "uart.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_ERASE_BOTH_APPROM_REGIONS (0U)

#define MAX_CHUNK_RETRIES (10U)
#define WORDS_PER_CHUNK_WITHOUT_CRC (512U)
#define BYTES_PER_CHUNK_WITHOUT_CRC                                            \
  (WORDS_PER_CHUNK_WITHOUT_CRC * sizeof(uint32_t))
#define WORDS_PER_CHUNK_WITH_CRC (513U)

typedef enum FWUpdateAbortReason {

  FW_UPDATE_ERR_UNEXPECTED_HANDSHAKE_PACKET,
  FW_UPDATE_ERR_IMAGE_METADATA_VERSION_MISMATCH,
  FW_UPDATE_ERR_IMAGE_METADATA_SIZE_MISMATCH,
  FW_UPDATE_ERR_IMAGE_METADATA_CRC_MISMATCH,
  FW_UPDATE_ERR_TOO_MANY_CHUNK_RETRIES,
  FW_UPDATE_ERR_UNEXPECTED_END_PACKET,
  FW_UPDATE_ERR_FINAL_IMAGE_CRC_MISMATCH,
  FW_UPDATE_ERR_IMAGE_TOO_LARGE,

} FWUpdateAbortReason;

static FirmwareInfo_t *fw = NULL;
// We use this to refer to the region we are writing in
volatile static AppromRegion targetAppromRegion;

// The current valid approm's region
volatile static AppromRegion runningAppromRegion;
// The approm region where our next update would go
volatile static AppromRegion nextAppromRegion;

/**** --- Firmware Metadata Functions --- ****/

static void ensure_fw_addr_initialized(void) {
  if (fw == NULL)
    fw = (FirmwareInfo_t *)FW_METADATA_ADDR;
}

static void ensure_fw_approm_region_selected(void) {
  ensure_fw_addr_initialized();
  runningAppromRegion = fw->region ? fw->region : APPROM_REGION_UNDEFINED;

  switch (runningAppromRegion) {

  case APPROM_REGION_UNDEFINED:
  case APPROM_REGION_B:
    nextAppromRegion = APPROM_REGION_A;
    break;
  case APPROM_REGION_A:
    nextAppromRegion = APPROM_REGION_B;
    break;
  }
}

AppromRegion get_existing_approm_region(void) {
  ensure_fw_approm_region_selected();
  return runningAppromRegion;
}

uint8_t get_approm_sector_start(AppromRegion region) {
  ensure_fw_approm_region_selected();
  switch (region) {
  case APPROM_REGION_UNDEFINED:
  case APPROM_REGION_A:
    return FLASH_APPROM_A_SECTOR_START;
  case APPROM_REGION_B:
    return FLASH_APPROM_B_SECTOR_START;
    break;
  default:
    return FLASH_APPROM_A_SECTOR_START;
  }
}

uint8_t get_approm_sector_end(AppromRegion region) {
  switch (region) {
  case APPROM_REGION_UNDEFINED:
  case APPROM_REGION_A:
    return FLASH_APPROM_A_SECTOR_END;
  case APPROM_REGION_B:
    return FLASH_APPROM_B_SECTOR_END;
    break;
  default:
    return FLASH_APPROM_A_SECTOR_END;
  }
}

uint32_t *get_approm_start_address(AppromRegion region) {
  switch (region) {
  case APPROM_REGION_UNDEFINED:
  case APPROM_REGION_A:
    return (uint32_t *)&__appromA_start__;
  case APPROM_REGION_B:
    return (uint32_t *)&__appromB_start__;
    break;
  default:
    return (uint32_t *)&__appromA_start__;
  }
}

AppromRegion get_next_approm_region(void) {
  ensure_fw_approm_region_selected();
  return nextAppromRegion;
}

const char *get_approm_region_name(AppromRegion region) {
  switch (region) {

  case APPROM_REGION_UNDEFINED:
    return "?";
  case APPROM_REGION_A:
    return "A";
  case APPROM_REGION_B:
    return "B";
  }
  return "\0";
}

void print_firmware_info(void) {
  ensure_fw_addr_initialized();

  char regionName[2];
  strcpy(regionName, get_approm_region_name(fw->region));

  printf("[:::::::: FIRMWARE INFO START ::::::::]\r\n");
  printf("  + Version:\t 0x%08" PRIX32 "\r\n  + CRC:\t 0x%08" PRIX32
         "\r\n  + Region:\t %s\r\n",
         fw->version, fw->crc, regionName);
  printf("[::::::::: FIRMWARE INFO END :::::::::]\r\n");
}

void update_firmware_info(FirmwareInfo_t *newFwInfo) {
  flash_sector_erase(METAROM_SECTOR);
  write_words_flash((uint32_t)FW_METADATA_ADDR, (uint32_t)newFwInfo,
                    FW_METADATA_WORDS);
}

void update_approm_regions(void) {
  runningAppromRegion = targetAppromRegion;

  switch (targetAppromRegion) {

  case APPROM_REGION_UNDEFINED: // Should never happen
  case APPROM_REGION_B:
    nextAppromRegion = APPROM_REGION_A;
    break;
  case APPROM_REGION_A:
    nextAppromRegion = APPROM_REGION_B;
    break;
  }
}

uint32_t *get_current_approm_start(void) {
  ensure_fw_approm_region_selected();
  AppromRegion region = get_existing_approm_region();
  return get_approm_start_address(region);
}

FirmwareInfo_t *get_firmware_info(void) {
  ensure_fw_addr_initialized();
  return fw;
}

/**** --- Firmware Write Functions --- ****/

void erase_approm_sectors(AppromRegion region) {

  wait_for_pending_flash_operations();

  if (region == APPROM_REGION_UNDEFINED)
    return;
  uint8_t sector_start = get_approm_sector_start(region);
  uint8_t sector_end = get_approm_sector_end(region);

  for (uint8_t i = sector_start; i <= sector_end; i++) {
    flash_sector_erase(i);
  }
}

void firmware_update_cleanup(bool updateSuccessful) {

  wait_for_pending_flash_operations();

  /* End Flash programming */
  flash_program_end();

  /* Re-lock write/erase operations on the flash */
  flash_lock();

  /* Re-enable echo on UART RX */
  uart_rx_enable_echo();

  if (updateSuccessful) {
    printf("[FIRMWARE] Update successful!\r\n");
    printf("[FIRMWARE] Approm region: %s\r\n",
           get_approm_region_name(targetAppromRegion));

    update_approm_regions();

  } else {
    printf("[FIRMWARE] Update failed!\r\n");
  }

  targetAppromRegion = APPROM_REGION_UNDEFINED;

  /* Deinitialize UART TXRX */
  uart_tx_rx_deinit();
}

void abort_firmware_update(FWUpdateAbortReason reason) {

  /* Send a signal to the host to signify something went wrong */
  uart_send_byte(FW_UPDATE_ERR);

  /* End flash writes, if begun */
  flash_program_end();

  /* Flash sector erase */
  erase_approm_sectors(targetAppromRegion);
  targetAppromRegion = APPROM_REGION_UNDEFINED;

  /* Print some debugging information */
  printf("[FIRMWARE] Update aborted! Reason: ");
  switch (reason) {
  case FW_UPDATE_ERR_UNEXPECTED_HANDSHAKE_PACKET:
    printf("Unxpected handshake packet sequence recieved. ");
    break;
  case FW_UPDATE_ERR_IMAGE_METADATA_VERSION_MISMATCH:
    printf("Metadata mismatch found for - Image Version. ");
    break;
  case FW_UPDATE_ERR_IMAGE_METADATA_SIZE_MISMATCH:
    printf("Metadata mismatch found for - Image Size. ");
    break;
  case FW_UPDATE_ERR_IMAGE_METADATA_CRC_MISMATCH:
    printf("Metadata mismatch found for - Image CRC. ");
    break;
  case FW_UPDATE_ERR_TOO_MANY_CHUNK_RETRIES:
    printf("Too many retries during chunk transfer. ");
    break;
  case FW_UPDATE_ERR_UNEXPECTED_END_PACKET:
    printf("Unexpected data found instead of end packet. ");
    break;
  case FW_UPDATE_ERR_FINAL_IMAGE_CRC_MISMATCH:
    printf(
        "Final calculated CRC of the image does not match provided metadata. ");
    break;
  case FW_UPDATE_ERR_IMAGE_TOO_LARGE:
    printf("Image size too large for the target approm region. ");
    break;
  }
  printf("\r\n");

  /* Send a signal to the host to say we finished sending the error message */
  uart_send_byte(FW_UPDATE_FIN);

  /* Call the housekeeping function for cleanup */
  firmware_update_cleanup(false);
}

/**
 * @brief Updates the app firmware via UART. Uses a custom protocol
 *        for the process: A handshake at the start (during which the
 *        target approm region is erased), then the metadata, and finally
 *        the firmware image in 512-word chunks. The host is responsible
 *        for padding the last chunk with `0xFF`s to preserve the reset/
 *        erased state of the flash. Once the transfer is complete, the
 *        firmware metadata will be written to the metarom region.
 *
 *        Protocol overview:
 *
 *        [Handshake]
 *          Bootloader  ->  Host : FW_UPDATE_REQ
 *          Host        ->  Boot : FW_UPDATE_ACK + FW_UPDATE_SYNC
 *          << Bootloader erases the target approm region >>
 *          Bootloader  ->  Host : FW_UPDATE_ACK
 *
 *        [Metadata] (repeated for version, size, and crc)
 *          Host        ->  Boot : <field_value> (word) + CRC32(<field_value>)
 *          Bootloader  ->  Host : FW_UPDATE_ACK  (or aborts on mismatch)
 *
 *        [Chunk Transfer] (repeated for each chunk)
 *          Host        ->  Boot : 512 words of image data + CRC32(512 words)
 *          Bootloader  ->  Host : FW_UPDATE_ACK   (chunk ok, send next)
 *                              or FW_UPDATE_RSND  (CRC mismatch, resend)
 *                              or aborts after MAX_CHUNK_RETRIES consecutive
 *                                 failures on the same chunk
 *
 *        [Finalisation]
 *          Host        ->  Boot : FW_UPDATE_FIN
 *          Bootloader verifies the full image CRC against the metadata CRC.
 *          On success, writes metadata to metarom and completes.
 *          On failure, aborts and erases the target approm region.
 *
 * @note The target approm region is selected automatically based on the
 *       currently running region stored in the firmware metadata:
 *         - No existing firmware  ->  Region A
 *         - Currently running A   ->  Region B
 *         - Currently running B   ->  Region A
 *
 * @note The image size is validated against the target region's capacity
 *       before the transfer begins. Aborts with
 *       FW_UPDATE_ERR_IMAGE_TOO_LARGE if the image does not fit.
 *
 * @returns True on a successful update, false otherwise.
 *
 */
bool update_firmware_via_uart(void) {

  targetAppromRegion = get_next_approm_region();

  uint32_t *appRomStart = get_approm_start_address(targetAppromRegion);
  uint32_t currChunk = 0;
  uint32_t *flashCursor = appRomStart;
  uint8_t res1, res2;
  uint32_t image_size = 0;
  uint32_t image_version = 0;
  uint32_t image_crc = 0;
  uint32_t image_words = 0;
  uint32_t num_chunks = 0;
  uint8_t chunk_retries = 0;
  uint32_t crc = 0;
  bool crc_valid = false;
  FirmwareInfo_t currFwInfo;

  systick_init();

  uint32_t chunks[WORDS_PER_CHUNK_WITH_CRC];

  /** UART Handshake - Required before Firmware update **/
  /* Step 0: Initialize UART for both TX RX if not already */
  uart_tx_rx_init();
  /* Step 1: Send a FW_UPDATE_REQUEST to host */
  uart_send_byte(FW_UPDATE_REQ);
  /* Step 2: Wait for an ACK + SYNC from host, use a timeout */
  uart_read_byte(&res1, -1);
  uart_read_byte(&res2, -1);

  if (!(res1 == FW_UPDATE_ACK && res2 == FW_UPDATE_SYNC)) {
    abort_firmware_update(FW_UPDATE_ERR_UNEXPECTED_HANDSHAKE_PACKET);
    return false;
  }

  /* Step 3: Disable UART input echo */
  uart_rx_disable_echo();

  /** Flash preparation **/
  /* Step 0: Get current firmware data, and check if firmware exists */
  FirmwareInfo_t *flashFw = get_firmware_info();
  if (flashFw->version != FW_UNDEFINED_VERSION) {
    /* Step 1: If it does exist, keep it in a local variable for later (?) */
    currFwInfo.version = flashFw->version;
    currFwInfo.size = flashFw->size;
    currFwInfo.crc = flashFw->crc;
  }
  /* Step 2: Erase the flash sector of the target approm region */
  flash_unlock();
  
  /* We can set this define to 1 so that all approm regions are wiped before flashing */
  if (DEBUG_ERASE_BOTH_APPROM_REGIONS) {
    erase_approm_sectors(APPROM_REGION_A);
    erase_approm_sectors(APPROM_REGION_B);
  } else {
    erase_approm_sectors(targetAppromRegion);
  }

  /* Step 3: Send an ACK to the host to initiate data transfer */
  uart_send_byte(FW_UPDATE_ACK);

  /** UART Data Transfer sequence **/
  /* Step 0: Host will send the image version (word), respond with ACK */
  uart_read_word(&image_version, -1);
  uart_read_word(&crc, -1);
  crc_valid = !(compute_crc32(&image_version, 1) ^ crc);
  if (!crc_valid) {
    abort_firmware_update(FW_UPDATE_ERR_IMAGE_METADATA_VERSION_MISMATCH);
    return false;
  }
  /* Valid image version received, proceed to next step */

  uart_send_byte(FW_UPDATE_ACK);

  /* Step 1: Host will send image size (word), respond with ACK */
  uart_read_word(&image_size, -1);
  uart_read_word(&crc, -1);
  crc_valid = !(compute_crc32(&image_size, 1) ^ crc);
  if (!crc_valid) {
    abort_firmware_update(FW_UPDATE_ERR_IMAGE_METADATA_SIZE_MISMATCH);
    return false;
  }
  uint32_t regionSize = (targetAppromRegion == APPROM_REGION_A)
                            ? (uint32_t)&__appromA_size__
                            : (uint32_t)&__appromB_size__;

  if (image_size > regionSize) {
    abort_firmware_update(FW_UPDATE_ERR_IMAGE_TOO_LARGE);
    return false;
  }

  /* Valid image size received, proceed to next step */

  /* image_words = ceil(image_size / size_per_word)*/
  image_words = (image_size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  num_chunks = (image_size + BYTES_PER_CHUNK_WITHOUT_CRC - 1) /
               BYTES_PER_CHUNK_WITHOUT_CRC;

  uart_send_byte(FW_UPDATE_ACK);

  /* Step 2: Host will send the image CRC (word), respond with ACK */
  uart_read_word(&image_crc, -1);
  uart_read_word(&crc, -1);
  crc_valid = !(compute_crc32(&image_crc, 1) ^ crc);
  if (!crc_valid) {
    abort_firmware_update(FW_UPDATE_ERR_IMAGE_METADATA_CRC_MISMATCH);
    return false;
  }

  /* Valid image crc received, proceed to next step */

  uart_send_byte(FW_UPDATE_ACK);

  /* Step 3: Repeat steps 4-8 until image is transferred */
  while (currChunk < num_chunks) {

    /* Step 4: Host will send 1 chunk of image (512 + 1 CRC) words */
    uart_read_word_stream(chunks, WORDS_PER_CHUNK_WITH_CRC, -1);

    crc_valid = validate_data_with_crc32(chunks, WORDS_PER_CHUNK_WITH_CRC);
    if (!crc_valid) {

      /* Abandon update if max chunk retries reached */
      if (++chunk_retries >= MAX_CHUNK_RETRIES) {
        abort_firmware_update(FW_UPDATE_ERR_TOO_MANY_CHUNK_RETRIES);
        return false;
      }

      /* Step 5: If CRC is NOT valid, it will send a RSND request to host */
      uart_send_byte(FW_UPDATE_RSND);

      /* Step 6: If host gets an RSND, it will send the chunk + CRC again */
      continue;

    } else {

      /* Step 7: For a valid chunk, commit to flash memory, and send an ACK*/
      write_words_flash((uint32_t)flashCursor, (uint32_t)chunks,
                        WORDS_PER_CHUNK_WITHOUT_CRC);
      /* Reset the retries per-chunk */
      chunk_retries = 0;
      flashCursor += WORDS_PER_CHUNK_WITHOUT_CRC;
      currChunk++;

      /* Step 8: If host gets an ACK, it will send the next chunk + its CRC */
      uart_send_byte(FW_UPDATE_ACK);
    }
  }

  /* Step 9: Host will send a FIN when data transfer is complete */
  uart_read_byte(&res1, -1);

  if (res1 != FW_UPDATE_FIN) {
    abort_firmware_update(FW_UPDATE_ERR_UNEXPECTED_END_PACKET);
    return false;
  }

  /* Step 10: Compute CRC of image, if it matches initial CRC, write metadata */
  crc = compute_crc32(appRomStart, image_words);
  if (crc != image_crc) {
    /* Step 11: If it does not match, throw an error, and wipe the sector */
    abort_firmware_update(FW_UPDATE_ERR_FINAL_IMAGE_CRC_MISMATCH);
    return false;
  }

  /** Firmware metadata write **/
  FirmwareInfo_t newFwInfo = (FirmwareInfo_t){
      .crc = image_crc,
      .size = image_size,
      .version = image_version,
      .region = targetAppromRegion,
  };

  update_firmware_info(&newFwInfo);

  /* Step 3: End the firmware update process */
  firmware_update_cleanup(true);

  /* On successful updates only */
  return true;
}