INCLUDE memory_map.ld

STACK_SIZE = 0x2000; /* 8k Bytes */

/* Section definitions - App */
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

    /* End of `.text` in bootrom/approm is where the `.data` source starts */
    _etext = .;
  } > approm


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