INCLUDE ../memory_map.ld

/**
**  FLASH memory is NOT byte-erasable, it needs to be erased sector by sector.
**  So a 16 kB size is perfect for bootrom, as it will use the first sector of the same size.
**  
**/

/* Section definitions - Bootloader */
SECTIONS
{

  .text :
  {
    /* Keep vector table sections */
    KEEP(*(.isr_vector .isr_vector.*))

    *(.text*)
    *(.rodata*)

    _etext = .;
  } > bootrom

}