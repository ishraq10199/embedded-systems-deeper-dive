INCLUDE memory_map.ld

/**
**  FLASH memory is NOT byte-erasable, it needs to be erased sector by sector.
**  The bootrom area for the bootloader will be using the first sector.
**  Sector 0 in the STM32F411xE is 16 kB in size, which makes it perfect for the bootloader.
**  Also, this linker script aligns each section boundary to 4 bytes.
**  This is so that when we copy over data to SRAM as words,
**  we do not touch other sections accidentally.
**  NOTE: When marking start and end, we use exactly the START of the next section,
**  e.g. after align start, or before align end.
**/

STACK_SIZE = 0x2000; /* 8k Bytes */

/* Section definitions - Bootloader */
SECTIONS
{

  .text :
  {
		. = ALIGN(4);
    ._stext = .;
    KEEP(*(.isr_vector .isr_vector.*))
    *(.text .text.*)
    *(.rodata .rodata.*)
		. = ALIGN(4);

    /* End of `.text` in bootrom/approm is where the `.data` source starts */
    _etext = .;
  } > bootrom

	.data	:	AT (_etext)
	{
		. = ALIGN(4);
		_sdata = .;
		*(.data .data.*)
		. = ALIGN(4);
		_edata = .;

	}	>	ram

	.bss	(NOLOAD)	:
	{
		. = ALIGN(4);
		_sbss = .;
		*(.bss .bss.*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;

	} > ram

	.stack (NOLOAD)	:
	{
		. = ALIGN(8);
    _sstack = .;
		. = . + STACK_SIZE;
		. = ALIGN(8);
    _estack = .;

	} > ram

  . = ALIGN(4);
  _end = .;

}