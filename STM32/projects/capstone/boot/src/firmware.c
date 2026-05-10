#include "firmware.h"
#include "crc32.h"
#include "flash.h"
#include "memory_map.h"
#include "tim.h"
#include "uart.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

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

} FWUpdateAbortReason;

typedef struct FirmwareInfo_t {

  uint32_t version;
  uint32_t size;
  uint32_t crc;

} FirmwareInfo_t;

static FirmwareInfo_t *fw = NULL;

void test(void) {
  printf("Hello firmware 0x%08" PRIX32 "\r\n", (uint32_t)FW_METADATA_ADDR);
  print_firmware_info();
}

/**** --- Firmware Metadata Functions --- ****/

static void ensure_fw_addr_initialized(void) {
  if (fw == NULL)
    fw = (FirmwareInfo_t *)FW_METADATA_ADDR;
}

void print_firmware_info(void) {
  ensure_fw_addr_initialized();
  printf("[:::::::: FIRMWARE INFO START ::::::::]\r\n");
  printf("  + Version:\t 0x%08" PRIX32 "\r\n  + CRC:\t 0x%08" PRIX32 "\r\n",
         fw->version, fw->crc);
  printf("[::::::::: FIRMWARE INFO END :::::::::]\r\n");
}

void update_firmware_info(FirmwareInfo_t *newFwInfo) {
  write_words_flash((uint32_t)FW_METADATA_ADDR, (uint32_t)newFwInfo,
                    FW_METADATA_WORDS);
}

FirmwareInfo_t *get_firmware_info(void) {
  ensure_fw_addr_initialized();
  return fw;
}

/**** --- Firmware Write Functions --- ****/

void firmware_update_cleanup(bool updateSuccessful) {

  /* End Flash programming */
  flash_program_end();

  /* Re-lock write/erase operations on the flash */
  flash_lock();

  /* Re-enable echo on UART RX */
  uart_rx_enable_echo();

  /**
  ** TODO: After A/B flash implementation, print that info here
  **       This will involve info regarding the current valid flash
  **       sector, the metadata of the last valid firmware, etc.
  */

  if (updateSuccessful) {
    printf("[FIRMWARE] Update successful!\r\n");
  } else {
    printf("[FIRMWARE] Update failed!\r\n");
  }

  /* Deinitialize UART TXRX */
  uart_tx_rx_deinit();
}

void abort_firmware_update(FWUpdateAbortReason reason) {

  /* End flash writes, if begun */
  flash_program_end();

  /* Flash sector erase */
  flash_sector_erase(FLASH_APPROM_SECTOR);

  /* Print some debugging information */
  printf("[FIRMWARE] Update aborted! Reason: ");
  switch (reason) {
  case FW_UPDATE_ERR_UNEXPECTED_HANDSHAKE_PACKET:
    printf("Unxpected handshake packet sequence recieved. ");
    break;
  case FW_UPDATE_ERR_IMAGE_METADATA_VERSION_MISMATCH:
    printf("Metadata mismatch found for: Image Version. ");
    break;
  case FW_UPDATE_ERR_IMAGE_METADATA_SIZE_MISMATCH:
    printf("Metadata mismatch found for: Image Size. ");
    break;
  case FW_UPDATE_ERR_IMAGE_METADATA_CRC_MISMATCH:
    printf("Metadata mismatch found for: Image CRC. ");
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
  }
  printf("\r\n");

  /* Call the housekeeping function for cleanup */
  firmware_update_cleanup(false);
}

/**
 * @brief Updates the app firmware via UART. Uses a custom protocol
 *        for the process: A handshake at the start, then send the
 *        metadata, and finally the firmware image in 512-word chunks
 *        the last chunk will be padded with `0xFF`s, to preserve the
 *        reset/erased state of the flash. Once the transfer is complete,
 *        the firmware metadata will be written on the ROM.
 *
 *        TODO: Document the full protocol here, or somewhere else.
 *
 */
void update_firmware_via_uart(void) {

  uint32_t currChunk = 0;
  uint32_t *appRomStart = (uint32_t *)&__approm_start__;
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
    return;
  }

  /* Step 3: Disable UART input echo */
  uart_rx_disable_echo();

  /** Flash preparation **/
  /* Step 0: Get current firmware data, and check if firmware exists */
  FirmwareInfo_t *flashFw = get_firmware_info();
  if (flashFw->version != 0xFFFFFFFF) {
    /* Step 1: If it does exist, keep it in a local variable for later (?) */
    currFwInfo.version = flashFw->version;
    currFwInfo.size = flashFw->size;
    currFwInfo.crc = flashFw->crc;
  }
  /* Step 2: Erase the flash sector of the approm region */
  flash_unlock();
  flash_sector_erase(FLASH_APPROM_SECTOR);
  flash_program_begin(FLASH_WRITE_SIZE_WORD);

  /* Step 3: Send an ACK to the host to initiate data transfer */
  uart_send_byte(FW_UPDATE_ACK);

  /** UART Data Transfer sequence **/
  /* Step 0: Host will send the image version (word), respond with ACK */
  uart_read_word(&image_version, -1);
  uart_read_word(&crc, -1);
  crc_valid = !(compute_crc32(&image_version, 1) ^ crc);
  if (!crc_valid) {
    abort_firmware_update(FW_UPDATE_ERR_IMAGE_METADATA_VERSION_MISMATCH);
    return;
  }
  /* Valid image version received, proceed to next step */

  uart_send_byte(FW_UPDATE_ACK);

  /* Step 1: Host will send image size (word), respond with ACK */
  uart_read_word(&image_size, -1);
  uart_read_word(&crc, -1);
  crc_valid = !(compute_crc32(&image_size, 1) ^ crc);
  if (!crc_valid) {
    abort_firmware_update(FW_UPDATE_ERR_IMAGE_METADATA_SIZE_MISMATCH);
    return;
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
    return;
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
        return;
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
    return;
  }

  /* Step 10: Compute CRC of image, if it matches initial CRC, write metadata */
  crc = compute_crc32(appRomStart, image_words);
  if (crc != image_crc) {
    /* Step 11: If it does not match, throw an error, and wipe the sector */
    abort_firmware_update(FW_UPDATE_ERR_FINAL_IMAGE_CRC_MISMATCH);
    return;
  }

  /** Firmware metadata write **/
  FirmwareInfo_t newFwInfo = (FirmwareInfo_t){
      .crc = image_crc,
      .size = image_size,
      .version = image_version,
  };

  update_firmware_info(&newFwInfo);

  /* Step 3: End the firmware update process */
  firmware_update_cleanup(true);

  /** TODO: Find a proper timeout value that works with the given baud-rate */

  /** EXTRA-TODO: Try to implement two approm regions to ping-pong
   *              write the new image to the inactive bank, verify it
   *              completely, then atomically switch the boot pointer.
   */
}