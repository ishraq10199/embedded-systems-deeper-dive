#include "memory_map.h"
#include <inttypes.h>

/**
 ** Load R1 into MSP (msr => Move immediate value to System Register)
 ** Branch to the address in r0 (bx => branch and exchange)
 **/
__attribute__((naked)) static void start_app(uint32_t pc, uint32_t sp) {
    __asm("             \n\
          msr msp, r1   \n\
          bx r0         \n\
    ");
}

/* This would go into .data's initializer varaibles, but if we exclude this from the binary, how will it work? */
volatile uint32_t myvar = 0x12345678;

int main(void) {
    /* Get the pointer to where the app is in ROM */
    uint32_t *app_code = (uint32_t *)__approm_start__;

    /* First 32-bit word contains the stack pointer init address */
    uint32_t app_sp = app_code[0];

    /* Second 32-bit word contains the address to the Reset Handler */
    uint32_t app_start = app_code[1];

    myvar++;

    /* Load the stack pointer to MSP and jump to the app reset handler for execution */
    start_app(app_start, app_sp);

    /* We never come here */
    while (1) {
    }
}