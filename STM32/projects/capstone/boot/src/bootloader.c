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

  /* Second 32-bit word contains the address to the Reset Handler */
  uint32_t app_start = app_code[1];

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
    }

    printf("[BOOTLOADER] Valid firmware not found.\r\n");
    printf("[BOOTLOADER] Sitting idle until next reset.\r\n");
  }

  /* We never come here */
  while (1) {
  }
}
