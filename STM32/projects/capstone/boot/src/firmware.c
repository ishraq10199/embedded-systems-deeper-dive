#include "firmware.h"
#include <inttypes.h>
#include <stdio.h>

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

FirmwareInfo_t *get_firmware_info(void) { return fw; }

/**** --- Firmware Write Functions --- ****/

void update_firmware_via_uart(void) {

  /** UART Handshake - Required before Firmware update **/
  /* Step 0: Initialize UART for both TX RX if not already */
  /* Step 1: Send a FW_UPDATE_REQUEST to host */
  /* Step 2: Wait for an SYNC + ACK from host, use a timeout */
  /* Step 3: When received, do the flash preparation */
  /* Step 4: Send an ACK to the host to initiate data transfer */

  /** Flash preparation **/
  /* Step 0: Get current firmware data, and check if firmware exists */
  /* Step 1: If it does exist, keep it in a local variable for later (?) */
  /* Step 2: Erase the flash sector of the approm region */

  /** UART Data Transfer sequence **/
  /* Step 0: Host will send the image version (word), respond with ACK */
  /* Step 1: Host will send the image CRC (word), respond with ACK */
  /* Step 2: Host will send image size (word), respond with ACK */
  /* Step 3: Host will send 1 chunk of image (512 B) + 1 CRC word */
  /* Step 4: If CRC is NOT valid, it will send a RSND request to host */
  /* Step 5: If CRC is valid, it will commit to flash memory, and send an ACK*/
  /* Step 6: f host gets an RSND, it will send the same chunk + CRC again */
  /* Step 7: If host gets an ACK, it will send the next chunk + its CRC */
  /* Step 8: Repeat steps 2-5 until image is transferred */
  /* Step 9: Host will send a FIN when data transfer is complete */
  /* Step 10: Compute CRC of image, if it matches initial CRC, write metadata */
  /* Step 11: If it does not match, throw an error, and wipe the flash sector */

  /** Firmware metadata write **/
  /* Step 0: Write firmware version */
  /* Step 1: Write firmware size in bytes */
  /* Step 2: Write firmware CRC */
  /* Step 3: End the firmware update process */
}