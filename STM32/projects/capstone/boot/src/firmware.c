#include "firmware.h"
#include "crc32.h"
#include "flash.h"
#include "memory_map.h"
#include "uart.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_CHUNK_RETRIES (10U)

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

FirmwareInfo_t *get_firmware_info(void) { return fw; }

/**** --- Firmware Write Functions --- ****/

void abort_firmware_update(void) {
  /** TODO: Wipe flash sector */
  /** TODO: Re-lock the flash */
  /** TODO: Print some error message */
  /** TODO: Deinitialize UART and also other modules if needed */
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
  uint32_t computed_crc = 0;
  bool crc_valid = false;
  FirmwareInfo_t currFwInfo;

  uint32_t chunks[513];

  /** UART Handshake - Required before Firmware update **/
  /* Step 0: Initialize UART for both TX RX if not already */
  uart_tx_rx_init();
  /* Step 1: Send a FW_UPDATE_REQUEST to host */
  uart_send_byte(FW_UPDATE_REQ);
  /* Step 2: Wait for an ACK + SYNC from host, use a timeout */
  uart_read_byte(&res1, -1);
  uart_read_byte(&res2, -1);

  if (!(res1 == FW_UPDATE_ACK && res2 == FW_UPDATE_SYNC)) {
    abort_firmware_update();
    return;
  }

  /* Step 3: Disable UART input echo */
  uart_rx_disable_echo();
  /* Step 4: Send an ACK to the host to initiate data transfer */
  uart_send_byte(FW_UPDATE_ACK);

  /** Flash preparation **/
  /* Step 0: Get current firmware data, and check if firmware exists */
  FirmwareInfo_t *fw = get_firmware_info();
  if (fw->version != 0xFFFFFFFF) {
    /* Step 1: If it does exist, keep it in a local variable for later (?) */
    currFwInfo.version = fw->version;
    currFwInfo.size = fw->size;
    currFwInfo.crc = fw->crc;
  }
  /* Step 2: Erase the flash sector of the approm region */
  flash_unlock();
  flash_sector_erase(FLASH_APPROM_SECTOR);
  flash_program_begin(FLASH_WRITE_SIZE_WORD);

  /** UART Data Transfer sequence **/
  /* Step 0: Host will send the image version (word), respond with ACK */
  uart_read_word(&image_version, -1);
  uart_send_byte(FW_UPDATE_ACK);

  /* Step 1: Host will send image size (word), respond with ACK */
  uart_read_word(&image_size, -1);
  image_words = (image_size + 3) / 4;
  num_chunks = (image_size + 2047) / 2048; /** TODO: Use symbolic values */
  uart_send_byte(FW_UPDATE_ACK);

  /* Step 2: Host will send the image CRC (word), respond with ACK */
  uart_read_word(&image_crc, -1);
  uart_send_byte(FW_UPDATE_ACK);

  /* Step 3: Repeat steps 4-8 until image is transferred */
  while (currChunk < num_chunks) {

    /* Step 4: Host will send 1 chunk of image (512 + 1 CRC) words */
    uart_read_word_stream(chunks, 513, -1);

    crc_valid = validate_data_with_crc32(chunks, 513);
    if (!crc_valid) {

      /* Abandon update if max chunk retries reached */
      if (++chunk_retries >= MAX_CHUNK_RETRIES) {
        abort_firmware_update();
        return;
      }

      /* Step 5: If CRC is NOT valid, it will send a RSND request to host */
      uart_send_byte(FW_UPDATE_RSND);

      /* Step 6: If host gets an RSND, it will send the chunk + CRC again */
      continue;

    } else {

      /* Step 7: For a valid chunk, commit to flash memory, and send an ACK*/
      write_words_flash((uint32_t)flashCursor, (uint32_t)chunks, 512);
      /* Reset the retries per-chunk */
      chunk_retries = 0;
      flashCursor += 512;
      currChunk++;

      /* Step 8: If host gets an ACK, it will send the next chunk + its CRC */
      uart_send_byte(FW_UPDATE_ACK);
    }
  }

  /* Step 9: Host will send a FIN when data transfer is complete */
  uart_read_byte(&res1, -1);

  if (res1 != FW_UPDATE_FIN) {
    abort_firmware_update();
    return;
  }

  /* Step 10: Compute CRC of image, if it matches initial CRC, write metadata */
  computed_crc = compute_crc32(appRomStart, image_words);
  if (computed_crc != image_crc) {
    /* Step 11: If it does not match, throw an error, and wipe the sector */
    abort_firmware_update();
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
  flash_program_end();

  /** Housekeeping **/
  uart_rx_enable_echo();
  uart_tx_deinit(); /** TODO: Consider moving this down */

  /** TODO: Find a proper timeout value that works with the given baud-rate */
  /** TODO: Use CRC checks for the image metadata values received from host */
  /** TODO: Make an abort function to break from the update process */
  /** TODO: Make a houskeeping function */
  /** TODO: Use the housekeeping function in both fail and success cases  */

  /** EXTRA-TODO: Try to implement two approm regions to ping-pong
   *              write the new image to the inactive bank, verify it
   *              completely, then atomically switch the boot pointer.
   */
}