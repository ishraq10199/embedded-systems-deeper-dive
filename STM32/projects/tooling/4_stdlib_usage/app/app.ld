INCLUDE memory_map.ld

STACK_SIZE = 0x2000; /* 8k Bytes */

/* Section definitions - App */
SECTIONS
{

  .text	:
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

		/* The executable code for the app lives in approm, but its to be executed from execram */
  } > execram AT > approm

	/* .ARM.exidx is sorted, so has to go in its own output section.  */
	PROVIDE_HIDDEN (__exidx_start = .);
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > bootrom
	PROVIDE_HIDDEN (__exidx_end = .);
	. = ALIGN(4);

  _etext = LOADADDR(.text) + SIZEOF(.text) + SIZEOF(.ARM.exidx);

	.data	: AT (_etext)
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