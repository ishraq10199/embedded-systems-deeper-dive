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
    _stext = .;
    KEEP(*(.isr_vector .isr_vector.*))
    *(.text .text.*)
    *(.rodata .rodata.*)
		. = ALIGN(4);

		/* Capture newlib runtime init/fini stubs (from crti.o, crtn.o) */
    KEEP(*(.init .init.*))
    KEEP(*(.fini .fini.*))

		/* Define this symbol at the current address, but only if nothing else has defined it */
		/* and don't export it publicly (PROVIDE_HIDDEN) - We need these for Newlib to work */

    /* C runtime init/fini arrays (used by newlib for library init) */
    . = ALIGN(4);
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);

    . = ALIGN(4);
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);

    . = ALIGN(4);
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);

  } > bootrom

	/* .ARM.exidx is sorted, so has to go in its own output section.  */
	/* If we do not define this section region ourselves, it may overlap with others */
	PROVIDE_HIDDEN (__exidx_start = .);
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > bootrom
	PROVIDE_HIDDEN (__exidx_end = .);
	. = ALIGN(4);

  _etext = LOADADDR(.text) + SIZEOF(.text) + SIZEOF(.ARM.exidx);
	
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