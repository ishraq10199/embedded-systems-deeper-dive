# Learning Embedded Systems

This repo will be used to track my learning progress with embedded systems. Although I have some experience with it, I never quite formalized my learning. Hopefully I can cover some stuff that I was lacking, and track my journey throughout.

## STM32

As a starter, I will be using an STM32 board, or more precisely, [this one](https://store.roboticsbd.com/development-boards/1133-xnucleo-f411re-improved-stm32-nucleo-board-robotics-bangladesh.html), as it was the one I had in stock when I was starting. The board is assumed to be deprecated, as looking up its datasheet returns a `404` on the manufacturer's site.

The documentation and datasheets will be included under the `docs` folder.

### Baremetal

This folder contains projects done without using the hardware abstraction layer (HAL). I wanted to learn how the baremetal code works by manipulating registers and values in memory addresses directly.

The resulting code may not be as versatile as one would find in the HAL libraries. For example, the ADXL345 library I wrote can access the data registers (raw IMU values) via SPI, but I did not write any function that can access specific data registers via SPI, like I did when writing the functions responsible for I2C access in the same library.

All in all, it turned out to be quite an educational and an enjoyable experience overall to sift through the datasheets and operate on baremetal hardware.

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



