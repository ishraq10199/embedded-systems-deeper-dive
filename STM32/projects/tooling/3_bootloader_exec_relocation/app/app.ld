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

		/* The executable code for the app lives in approm, but its to be executed from execram */
  } > execram AT > approm

  /* End of `.text` cannot be put inside .text like before */
	/* This is because it would attain an address inside execram (VMA) */
	/* So we need to manually tell _etext to start at the end of approm */
	/* LOADADDR(.text) specifically gives us the LMA */
  _etext = LOADADDR(.text) + SIZEOF(.text);

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