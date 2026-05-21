#include "firmware.h"
#include "stm32f411xe.h"
#include "tim.h"
#include "uart.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define FW_UPDATE_REQ_TIMEOUT_MS (5000U)

/**
 ** Load R1 into MSP (msr => Move immediate value to System Register)
 ** Branch to the address in r0 (bx => branch and exchange)
 **/
__attribute__((naked)) static void start_app(uint32_t pc, uint32_t sp) {
  __asm("               \n\
          msr msp, r1   \n\
          bx r0         \n\
    ");
}

void jump_to_app_firmware(void) {

  printf("[BOOTLOADER] Valid app firmware found! Starting app...\r\n");

  uart_rx_enable_echo();
  uart_tx_rx_deinit();

  /* Get the pointer to where the app is in ROM */
  uint32_t *app_code = get_current_approm_start();

  /* First 32-bit word contains the stack pointer init address */
  uint32_t app_sp = app_code[0];

  /* Some logic is needed to get the ACTUAL jump address of the app */
  /**
   ** Here, X is where the contiguous app regions start. This space [ ... ] represents both regions.
   ** Y is the start address of our current app firmware in flash, stored as a value in `app_code`
   ** Z is the reset handler's actual location, stored in `app_code[1]`
   ** B is where region B starts
   ** 
   ** --- WHEN FIRMWARE IS FLASHED TO REGION A ---
   ** 
   **          [      |     |      |                                    ]
   **          X      Y     Z      B
   ** 
   ** The problem arises when firmware is in region B. Now, the X is fixed, as its a hard-coded address.
   ** i.e. X is always where the contiguous approm regions start, the beginning of `appromA_start`.
   ** Y is also where it should be, and its value is an address in RAM
   ** The problem is Z. It now points to a reset handler "as if the firmware is in region A" (Z*)
   ** What it should be: An address to the reset handler "as if the firmware is in region B" (Z)
   ** 
   ** --- WHEN FIRMWARE IS FLASHED TO REGION B ---
   **
   ** 
   **          [                   |      |     |                       ]
   **          X                   B      Y     Z
   **                       |
   **                       Z*
   **                       ^
   **      (where it thinks the reset handler is) 
   **
   ** So what we do is get the offset between the start of the contiguous approm regions (X) and Z*.
   ** We make sure to not take the thumb-bit for Z*, as it will interfere with offset calculation.
   ** So, offset difference = (Z* without thumb-bit) - X
   ** Now we add this offset with B, i.e. corrected = B + offset_difference
   ** Then we put the thumb-bit back into the corrected address, i.e. app_start = corrected | thumb_bit
   **
   */

  /* We need to temporarily get rid of the thumb-bit to calculate the offset */
  /* And then add the thumb-bit in, when the offset is corrected */

  /* Get the actual base address of all (contiguous) approm regions */
  uint32_t full_approm_base_addr = (uint32_t )&__appromA_start__;
  uint32_t current_app_region_base_addr = (uint32_t)app_code;

  /* Store the thumb bit for later */
  uint8_t app_start_thumb_bit = app_code[1] & 1U;

  /* Temporarily discard the thumb bit and get the offset difference */
  uint32_t app_start_offset = (app_code[1] & ~(1U)) - full_approm_base_addr;

  /* Calculate the correct app start address */
  uint32_t corrected_app_start = (current_app_region_base_addr + app_start_offset) | app_start_thumb_bit;
  
  /* Second 32-bit word contains the address to the corrected Reset Handler address */
  uint32_t app_start = corrected_app_start;

  // Disable all interrupts before jumping
  __disable_irq();

  // Disable SysTick and clear its exception pending bit
  systick_deinit();
  SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;

  // Relocate the vector table to the application's vector table
  SCB->VTOR = (uint32_t)app_code;

  /* Load stack pointer to MSP and jump to app reset handler for execution */
  start_app(app_start, app_sp);
}

int main(void) {

  /* UART message out */
  uart_tx_rx_init();

  /* We wait for a host to send an update notice */

  /* Required for the update notice recieve timeout to work */
  systick_init();

  uint8_t received_byte;
  bool update_successful = false;

  /* Needed, so that our recieved characters do not get echoed to the host */
  uart_rx_disable_echo();

  int res = uart_read_byte(&received_byte, FW_UPDATE_REQ_TIMEOUT_MS);

  if (res != -1 && received_byte == FW_UPDATE_SYNC) {

    /* If an update notice was recieved, we respond with an ACK */
    uart_send_byte(FW_UPDATE_ACK);

    /* We reset the states to default before proceeding with the update */
    uart_rx_enable_echo();
    systick_deinit();

    /* Finally, we start the update process */
    update_successful = update_firmware_via_uart();

    if (update_successful) {
      uart_tx_rx_init();
      printf("[BOOTLOADER] Updated approm found. Details:\r\n");
      print_firmware_info();
      jump_to_app_firmware();
    }

  } else {
    /* Otherwise, we resume the usual bootloader tasks */
    printf("********************************\r\n");
    printf("**** BOOTLOADER v4 SAYS HI! ****\r\n");
    printf("********************************\r\n");
    printf("\r\n");

    printf("[BOOTLOADER] Checking if valid firmware exists...\r\n");

    FirmwareInfo_t *fw = get_firmware_info();

    /* Valid firmware was found, so we start that */
    if (fw != NULL && fw->region != APPROM_REGION_UNDEFINED) {
      printf("[BOOTLOADER] Approm found. Details:\r\n");
      print_firmware_info();
      jump_to_app_firmware();

    } else {
      /* Separete else block to not come here if the jump fails */
      printf("[BOOTLOADER] Valid firmware not found.\r\n");
      printf("[BOOTLOADER] Sitting idle until next reset.\r\n");
    }
    
  }

  /* We never come here */
  while (1) {
  }
}
