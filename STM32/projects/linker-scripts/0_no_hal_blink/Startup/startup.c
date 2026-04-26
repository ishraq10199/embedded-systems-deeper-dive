#include <stdint.h>

/* SRAM => 512Kbytes, starting at 0x2000_0000 */
#define SRAM_START              (0x20000000U)
#define SRAM_SIZE               (512U * 1024U)
#define SRAM_END                (SRAM_START + SRAM_SIZE)

/* Stack Pointer Init address is at the start of the vector table */
#define SP_INIT_ADDR            (SRAM_END)

/* Vector table size in words */
#define ISR_VECTOR_SIZE_WORDS   (102U)

/* The ISR Vector Table */

void Default_Handler(void);
void Reset_Handler(void);

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void WWDG_IRQHandler(void);
void PVD_IRQHandler(void);
void TAMP_STAMP_IRQHandler(void);
void RTC_WKUP_IRQHandler(void);
void FLASH_IRQHandler(void);
void RCC_IRQHandler(void);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void DMA1_Stream0_IRQHandler(void);
void DMA1_Stream1_IRQHandler(void);
void DMA1_Stream2_IRQHandler(void);
void DMA1_Stream3_IRQHandler(void);
void DMA1_Stream4_IRQHandler(void);
void DMA1_Stream5_IRQHandler(void);
void DMA1_Stream6_IRQHandler(void);
void ADC_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void TIM1_BRK_TIM9_IRQHandler(void);
void TIM1_UP_TIM10_IRQHandler(void);
void TIM1_TRG_COM_TIM11_IRQHandler(void);
void TIM1_CC_IRQHandler(void);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void I2C2_EV_IRQHandler(void);
void I2C2_ER_IRQHandler(void);
void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void RTC_Alarm_IRQHandler(void);
void OTG_FS_WKUP_IRQHandler(void);
void DMA1_Stream7_IRQHandler(void);
void SDIO_IRQHandler(void);
void TIM5_IRQHandler(void);
void SPI3_IRQHandler(void);
void DMA2_Stream0_IRQHandler(void);
void DMA2_Stream1_IRQHandler(void);
void DMA2_Stream2_IRQHandler(void);
void DMA2_Stream3_IRQHandler(void);
void DMA2_Stream4_IRQHandler(void);
void OTG_FS_IRQHandler(void);
void DMA2_Stream5_IRQHandler(void);
void DMA2_Stream6_IRQHandler(void);
void DMA2_Stream7_IRQHandler(void);
void USART6_IRQHandler(void);
void I2C3_EV_IRQHandler(void);
void I2C3_ER_IRQHandler(void);
void FPU_IRQHandler(void);
void SPI4_IRQHandler(void);
void SPI5_IRQHandler(void);

__attribute__((weak, alias("Default_Handler"))) void NMI_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void HardFault_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void MemManage_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void BusFault_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void UsageFault_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void SVC_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void DebugMon_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void PendSV_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void SysTick_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void WWDG_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void PVD_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void TAMP_STAMP_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void RTC_WKUP_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void FLASH_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void RCC_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void EXTI0_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void EXTI1_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void EXTI2_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void EXTI3_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void EXTI4_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA1_Stream0_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA1_Stream1_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA1_Stream2_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA1_Stream3_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA1_Stream4_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA1_Stream5_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA1_Stream6_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void ADC_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void EXTI9_5_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void TIM1_BRK_TIM9_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void TIM1_UP_TIM10_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void TIM1_TRG_COM_TIM11_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void TIM1_CC_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void TIM2_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void TIM3_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void TIM4_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void I2C1_EV_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void I2C1_ER_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void I2C2_EV_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void I2C2_ER_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void SPI1_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void SPI2_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void USART1_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void USART2_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void EXTI15_10_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void RTC_Alarm_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void OTG_FS_WKUP_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA1_Stream7_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void SDIO_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void TIM5_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void SPI3_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA2_Stream0_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA2_Stream1_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA2_Stream2_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA2_Stream3_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA2_Stream4_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void OTG_FS_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA2_Stream5_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA2_Stream6_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void DMA2_Stream7_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void USART6_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void I2C3_EV_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void I2C3_ER_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void FPU_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void SPI4_IRQHandler(void);
__attribute__((weak, alias("Default_Handler"))) void SPI5_IRQHandler(void);

/* Vector table definitions */
/* Array of const pointers to void functions */
void (* const isr_vector[ISR_VECTOR_SIZE_WORDS])(void) __attribute__((section(".isr_vector"))) = {

 /* Stack pointer init */
 (void (*)(void))SP_INIT_ADDR,

 /* Cortex M4 System Exceptions */
 Reset_Handler,
 NMI_Handler,
 HardFault_Handler,
 MemManage_Handler,
 BusFault_Handler,
 UsageFault_Handler,
 0, 0, 0, 0,
 SVC_Handler,
 DebugMon_Handler,
 0,
 PendSV_Handler,
 SysTick_Handler,

 /* STM32F411xE Interrupt Handlers */
 WWDG_IRQHandler,
 PVD_IRQHandler,
 TAMP_STAMP_IRQHandler,
 RTC_WKUP_IRQHandler,
 FLASH_IRQHandler,
 RCC_IRQHandler,
 EXTI0_IRQHandler,
 EXTI1_IRQHandler,
 EXTI2_IRQHandler,
 EXTI3_IRQHandler,
 EXTI4_IRQHandler,
 DMA1_Stream0_IRQHandler,
 DMA1_Stream1_IRQHandler,
 DMA1_Stream2_IRQHandler,
 DMA1_Stream3_IRQHandler,
 DMA1_Stream4_IRQHandler,
 DMA1_Stream5_IRQHandler,
 DMA1_Stream6_IRQHandler,
 ADC_IRQHandler,
 0, 0, 0, 0,
 EXTI9_5_IRQHandler,
 TIM1_BRK_TIM9_IRQHandler,
 TIM1_UP_TIM10_IRQHandler,
 TIM1_TRG_COM_TIM11_IRQHandler,
 TIM1_CC_IRQHandler,
 TIM2_IRQHandler,
 TIM3_IRQHandler,
 TIM4_IRQHandler,
 I2C1_EV_IRQHandler,
 I2C1_ER_IRQHandler,
 I2C2_EV_IRQHandler,
 I2C2_ER_IRQHandler,
 SPI1_IRQHandler,
 SPI2_IRQHandler,
 USART1_IRQHandler,
 USART2_IRQHandler,
 0,
 EXTI15_10_IRQHandler,
 RTC_Alarm_IRQHandler,
 OTG_FS_WKUP_IRQHandler,
 0, 0, 0, 0,
 DMA1_Stream7_IRQHandler,
 0,
 SDIO_IRQHandler,
 TIM5_IRQHandler,
 SPI3_IRQHandler,
 0, 0, 0, 0,
 DMA2_Stream0_IRQHandler,
 DMA2_Stream1_IRQHandler,
 DMA2_Stream2_IRQHandler,
 DMA2_Stream3_IRQHandler,
 DMA2_Stream4_IRQHandler,
 0, 0, 0, 0, 0, 0,
 OTG_FS_IRQHandler,
 DMA2_Stream5_IRQHandler,
 DMA2_Stream6_IRQHandler,
 DMA2_Stream7_IRQHandler,
 USART6_IRQHandler,
 I2C3_EV_IRQHandler,
 I2C3_ER_IRQHandler,
 0, 0, 0, 0, 0, 0, 0,
 FPU_IRQHandler,
 0, 0,
 SPI4_IRQHandler,
 SPI5_IRQHandler,
};

/* Externally defined - we need this to zero out uninitialized data
 *
 * During startup, the init values from flash memory is copied from
 * _extext onwards, to the SRAM's _sdata onwards.
 *
 * _etext => end   of .text (FLASH) => init value src for .data here
 * _sdata => start of .data (SRAM)  => init value dest (begin)
 * _edata => end   of .data (SRAM)  => init value dest (end)
 * _sbss  => start of .bss  (SRAM)  => zero out (begin)
 * _ebss  => end   of .bss  (SRAM)  => zero out (end)
 *
 */
extern uint32_t _etext, _sdata, _edata, _sbss, _ebss;


/* Defined functions */

void main(void);

void Reset_Handler(void) {
 /* Quirk:
  * _etext (and any similar) value is garbage, so we don't use it
  * Instead, we use `&_etext` to get the address we need 
  * The linker uses _etext's address as the relevant value
  * Details: https://sourceware.org/binutils/docs/ld/Source-Code-Reference.html
  */
 uint32_t *init_values = &_etext;
 uint32_t *data_dest = &_sdata;
 uint32_t data_section_size = (uint32_t)&_edata - (uint32_t)&_sdata;

 /* Copy init values from flash memory to SRAM */
 for (int i = 0; i < data_section_size; i++) {
  data_dest[i] = init_values[i];
 }

 /* Fill the .bss section with zero */
 for (uint32_t *bss = &_sbss; bss < &_ebss;) {
  *bss++ = 0;
 }

 /* Branch to main function */
 main();

 /* In case main ever returns, we fall into this infinite loop */
 while (1);

}


/* Fallback Exception/Interrupt Handler */
/* If any handler is defined elsewhere, that will take precedence over this one */
void Default_Handler(void) {
 /* We stay here indefinitely */
 while(1);
}
