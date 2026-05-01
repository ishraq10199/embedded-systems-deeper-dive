# Embedded Systems - Deeper Dive

This repo will be used to track my deep-dive learning progress with embedded systems. Although I have some experience with it, I never quite formalized my learning through first principles. Hopefully I can cover stuff I was always interested in, like linkers, bootloaders, baremetal code, RTOS, etc. and track my journey throughout.

## STM32

As a starter, I will be using an STM32 board, or more precisely, [this one](https://store.roboticsbd.com/development-boards/1133-xnucleo-f411re-improved-stm32-nucleo-board-robotics-bangladesh.html), as it was the one I had in stock when I was starting. The board is assumed to be deprecated, as looking up its datasheet returns a `404` on the manufacturer's site.

The documentation and datasheets will be included under the `docs` folder.

### Baremetal

This folder contains projects done without using the hardware abstraction layer (HAL). I wanted to learn how the baremetal code works by manipulating registers and values in memory addresses directly.

The resulting code may not be as versatile as one would find in the HAL libraries. For example, the ADXL345 library I wrote can access the data registers (raw IMU values) via SPI, but I did not write any function that can access specific data registers via SPI, like I did when writing the functions responsible for I2C access in the same library.

<details>

<summary>Project Detals (toggle expand)</summary>

| Project                                                                                      | Summary                                                                                                                           |
| -------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| [0 - LED Blink Simple](STM32/projects/baremetal/0_led_blink_simple)                          | Blinks LED1 (PA5) by directly computing and dereferencing peripheral memory addresses. No CMSIS headers used.                     |
| [1 - LED Blink Struct](STM32/projects/baremetal/1_led_blink_struct)                          | Same blink on LED2 (PC9), but register access is done through manually defined C structs mirroring the peripheral layout.         |
| [2 - GPIO Output](STM32/projects/baremetal/2_gpio_output)                                    | Blinks LED1 using CMSIS headers (`stm32f4xx.h`) instead of raw address defines. Intro to using the CMSIS device layer.            |
| [3 - GPIO BSRR](STM32/projects/baremetal/3_gpio_bsrr)                                        | Blinks LED1 using the GPIO Bit Set/Reset Register (BSRR) for atomic pin control, instead of read-modify-write on ODR.             |
| [4 - GPIO Input](STM32/projects/baremetal/4_gpio_input)                                      | Reads the onboard button (PC13) and drives LED1 accordingly. Intro to configuring a GPIO pin as input.                            |
| [5 - UART TX](STM32/projects/baremetal/5_uart_tx)                                            | Transmits a single character over USART2 by manually configuring baud rate, word length, and the TX enable bits.                  |
| [6 - UART Printf](STM32/projects/baremetal/6_uart_printf)                                    | Retargets `printf` to USART2 by implementing `__io_putchar`, enabling formatted output over serial.                               |
| [7 - UART Modular](STM32/projects/baremetal/7_uart_modular)                                  | Refactors UART init and transmit into a reusable `uart.c`/`uart.h` module used across all subsequent projects.                    |
| [8 - UART TX/RX](STM32/projects/baremetal/8_uart_tx_rx)                                      | Adds receive support to the UART module. Reads a character from USART2 and echoes it back.                                        |
| [9 - ADC Single Conversion](STM32/projects/baremetal/9_adc_single_conversion)                | Configures ADC1 on PA1 for single software-triggered conversions, polling the EOC flag and printing the result.                   |
| [10 - ADC Continuous Conversion](STM32/projects/baremetal/10_adc_continuous_conversion)      | Runs ADC1 in continuous mode, reading and printing values in a tight loop without re-triggering each conversion.                  |
| [11 - SysTick Timer](STM32/projects/baremetal/11_systick_timer)                              | Implements a blocking `systickDelayMs` function using the Cortex-M4 SysTick counter to blink the LED at a precise rate.           |
| [12 - Timer Basics](STM32/projects/baremetal/12_timer_basics)                                | Configures TIM2 to overflow at 1 Hz by setting prescaler and ARR, polling the UIF flag in the main loop.                          |
| [13 - Timer Output Compare](STM32/projects/baremetal/13_timer_output_compare)                | Uses TIM2 in output compare mode to automatically toggle the LED pin on match, with no polling in the main loop.                  |
| [14 - Timer Input Capture](STM32/projects/baremetal/14_timer_input_capture)                  | TIM2 drives PA5 via output compare; TIM3 captures the rising edge on PA6 (wired to PA5) and prints the timestamp.                 |
| [15 - Input Interrupt](STM32/projects/baremetal/15_input_interrupt)                          | Configures an EXTI line on PC13 (onboard button) to trigger an interrupt, printing a message in the ISR callback.                 |
| [16 - UART Interrupt](STM32/projects/baremetal/16_uart_interrupt)                            | Enables the USART2 RXNE interrupt. Received characters are handled in the IRQ handler, toggling the LED on `'1'`.                 |
| [17 - ADC Interrupt](STM32/projects/baremetal/17_adc_interrupt)                              | Runs ADC1 in continuous mode with EOC interrupt enabled, reading and printing the conversion result from the ISR.                 |
| [18 - SysTick Interrupt](STM32/projects/baremetal/18_systick_interrupt)                      | Configures SysTick to fire at 1 Hz. The `SysTick_Handler` toggles the LED and prints a message, freeing the main loop.            |
| [19 - Timer Interrupt](STM32/projects/baremetal/19_timer_interrupt)                          | Moves TIM2 overflow handling into `TIM2_IRQHandler`, toggling the LED and printing from the ISR instead of polling.               |
| [20 - DMA UART TX](STM32/projects/baremetal/20_dma_uart_tx)                                  | Transfers a string to USART2's data register via DMA1 Stream 6, with a transfer-complete IRQ that lights the LED.                 |
| [21 - I2C ADXL345](STM32/projects/baremetal/21_i2c_adxl345)                                  | Communicates with the ADXL345 accelerometer over I2C. Reads raw X/Y/Z registers and converts them to g values.                    |
| [22 - SPI ADXL345](STM32/projects/baremetal/22_spi_adxl346)                                  | Re-implements ADXL345 communication over SPI. The same `adxl345` module is extended to support both interfaces.                   |
| [23 - HardFault: Div by Zero](STM32/projects/baremetal/23_hardfault_ufsr_div_zero)           | Enables the DIV_0_TRP bit in SCB->CCR, then deliberately divides by zero. A custom fault handler catches and reports the UFSR.    |
| [24 - HardFault: Unaligned Access](STM32/projects/baremetal/24_hardfault_ufsr_unaligned_mem) | Enables UNALIGNED_TRP and accesses a buffer at non-word-aligned offsets to trigger a HardFault, caught by the same fault handler. |

</details>

<!-- <br> -->

#### Parting thoughts:

All in all, it turned out to be quite an educational and an enjoyable experience overall to sift through the datasheets and operate on baremetal hardware. Without relying on the abstractions, it gave me a better grasp on how the HAL works to some extent.

### RTOS

This folder contains 11 projects built using FreeRTOS (via CMSIS-RTOS2 and STM32 HAL), progressively covering the core concepts of real-time operating systems, from basic task scheduling and inter-task communication, through synchronization primitives and interrupt-driven designs, to classic concurrency problems like deadlock and priority inversion.

<details>

<summary>Project Detals</summary>

| Project                                                                         | Summary                                                                                                                                                                                |
| ------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [0 - UART Blink](STM32/projects/rtos/0_uart_blink)                              | Two concurrent tasks: one blinks an LED, the other reads a delay value over UART with `scanf`. Intro to the FreeRTOS scheduler.                                                        |
| [1 - Memory Management](STM32/projects/rtos/1_memory_management)                | Heap allocation with `pvPortMalloc`/`vPortFree`. Two tasks share a heap-allocated buffer, synchronized via a volatile flag.                                                            |
| [2 - Queues](STM32/projects/rtos/2_queues)                                      | Bidirectional inter-task messaging using two FreeRTOS queues, one carrying integer delay values, the other carrying status strings.                                                    |
| [3 - Mutex](STM32/projects/rtos/3_mutex)                                        | Two tasks race to increment a shared variable. A mutex guards the critical section, demonstrating how it prevents the resulting race condition.                                        |
| [4 - Semaphore](STM32/projects/rtos/4_semaphore)                                | Producer-consumer with counting semaphores tracking buffer fullness/emptiness. Multiple dynamically-created producer and consumer tasks.                                               |
| [5 - Software Timers](STM32/projects/rtos/5_software_timers)                    | Creates a one-shot and a recurring FreeRTOS software timer, both sharing a single callback and distinguished by their timer ID.                                                        |
| [6 - Timer Example](STM32/projects/rtos/6_timer_example)                        | Practical use of a one-shot timer: an inactivity timer that turns off the LED 2 seconds after the last UART keystroke.                                                                 |
| [7 - Interrupts and Timers](STM32/projects/rtos/7_interrupt_and_timers)         | A hardware timer ISR increments a counter. A task reads it safely using `taskENTER_CRITICAL` and its ISR-safe counterpart.                                                             |
| [8 - Interrupts and Semaphores](STM32/projects/rtos/8_interrupt_and_semaphores) | ADC triggered by hardware timer, ISR fills a double buffer. A semaphore gates buffer swaps; a calc task is woken via task notification; a CLI task exposes an `avg` command over UART. |
| [9 - Deadlocks and Starvation](STM32/projects/rtos/9_deadlocks_and_starvation)  | Dining philosophers problem with 5 tasks. Deadlock is avoided by enforcing a resource ordering rule, always acquire the lower-numbered chopstick mutex first.                          |
| [10 - Priority Inversion](STM32/projects/rtos/10_priority_inversion)            | Three tasks at different priority levels share a mutex. Task startup is sequenced to deliberately trigger priority inversion and observe its effects.                                  |

</details>

#### Parting thoughts:

It was great to finally explore an RTOS, and I can now visualize how in past cases, using this would have helped me better organize projects. Although I have worked with concurrency before, I have not used it on microcontrollers or machines with similar constraints. Loads of fun!

### Tooling

This folder contains 5 projects exploring the build toolchain and bootloader development for the STM32F411RE from first principles, using hand-written Makefiles, custom linker scripts, and no IDE-generated glue code. The projects build progressively toward a fully functional multi-stage bootloader with shared memory, execution relocation, and standard library support. I did this part after going through the excellent blog posts in - [Zero to main()](https://interrupt.memfault.com/tag/zero-to-main/) by François Baldassari.

I decided to use the knowledge I gained there and learned a bit more stuff from various other sources to make similar implementations on the STM32 board I had. Quite a few things differ in this implementation, such as the memory maps, Makefile structure, project organization, etc.

<details>

<summary>Project Detals</summary>

| Project                                                                                           | Summary                                                                                                                                                                                                                                                                                                                                                       |
| ------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [0 - Minimal Blink with CMSIS Headers](STM32/projects/tooling/0_minimal_blink_with_cmsis_headers) | Hand-written Makefile that compiles a baremetal LED blink using the ARM GCC toolchain and CMSIS headers directly, without any IDE or HAL. Establishes the baseline build setup.                                                                                                                                                                               |
| [1 - Custom Bootloader](STM32/projects/tooling/1_custom_bootloader)                               | Flash is partitioned into a 16 kB bootrom and a 240 kB approm via a custom linker script. The bootloader prints a greeting over UART, reads the app's vector table from flash, loads the initial stack pointer into MSP, and jumps to the app's Reset Handler. A top-level Makefile builds both binaries and concatenates them into a single flashable image. |
| [2 - Bootloader Shared Data](STM32/projects/tooling/2_bootloader_shared_data)                     | Carves out a dedicated 4 kB `shared` region in SRAM that is not cleared by the startup code, allowing data to persist across resets. A `SharedData` struct tracks a boot counter that the bootloader increments on each startup, warns over UART if it exceeds 3, and resets it. The app binary can read the same shared region.                              |
| [3 - Bootloader Exec Relocation](STM32/projects/tooling/3_bootloader_exec_relocation)             | The bootloader copies the app binary from flash into a dedicated `execram` region in SRAM before jumping to it, so the application executes entirely from RAM rather than flash. Demonstrates how to structure linker regions and perform the copy loop using linker-exported symbols.                                                                        |
| [4 - Stdlib Usage](STM32/projects/tooling/4_stdlib_usage)                                         | Integrates newlib into the baremetal build by implementing the required `syscalls.c` stubs (`_write`, `_sbrk`, etc.) and retargeting `__io_putchar` to USART2. This enables `printf` for formatted output in the bootloader and replaces the manual copy loop with `memcpy` from `<string.h>`.                                                                |

</details>

#### Parting thoughts:

I was able to gain much deeper understanding of the core C build system, how the linker scripts and toolchain work, how embedded systems make use of the standard library, how memory regions can be manually mapped and used, and much more. In fact, I can now better appreciate how each byte of memory is laid out in memory, whenever I write code, and will probably always be thinking of efficient usage of the hardware.

### Notes

#### Using OpenOCD to debug

On a Fedora system which I used for this project, here are the steps I took for debugging with OpenOCD:

```
# Install OpenOCD
sudo dnf install openocd

# Ensure gdb is installed
which gdb

# Start the openocd instance
openocd -f /usr/share/openocd/scripts/interface/stlink-v2.cfg -f /usr/share/openocd/scripts/target/stm32f4x.cfg

# The above will open some ports (e.g. 3333)
# Start a gdb instance
gdb ./path/to/the/debug/elf/file/example-blink-led.elf

# Once started, we need to tell gdb to debug a remote target
(gdb) target extended-remote localhost:3333

# We can then debug the program using gdb :)

```

Example set commands in GDB to toggle `PC9` (`LED2`):

```
# (only once) Enable clock access to GPIOC in RCC_AHB1ENR
set *((uint32_t *)0x40023830) |= 0x00000005

# (only once) Set PC9 to output mode in GPIOC_MODER (bit 19 to 0 and bit 18 to 1)
set *((uint32_t *)0x40020800) &= ~0x00080000
set *((uint32_t *)0x40020800) |= 0x00040000

# (as many times as we want) Toggle bit 9 of GPIOC_ODR
set *((uint32_t *)0x40020814) ^= 0x00000200
```

To toggle the LED2, we can just run the last `set` command above. We don't need to run the first 3 `set` commands, as they are just for initializing `PC9`.
