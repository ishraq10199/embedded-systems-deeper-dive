ENTRY(Reset_Handler)

/* Memory definitions */
MEMORY
{
	rom			(rx)	:	ORIGIN = 0x08000000, LENGTH = 512K
	ram			(rwx)	:	ORIGIN = 0x20000000, LENGTH = 128K
}

STACK_SIZE = 0x2000; /* 8k Bytes */

SECTIONS
{
	.text	:
	{
			/* Prevent gc from removing the .isr_vector section from the output */
			/* The section name can be checked by doing an objdump -h on the Startup/startup*.o file */
			KEEP(*(.isr_vector*))

			/* Structure: <filename>(section), so the first * matches all filenames */
		  /* All the sections starting with .text => (.text*) */

			/* Put all the .text* sections here */
			*(.text*)

			/* Put all the .rodata* sections here */
			*(.rodata*)

			/* End of .text - For external use */
			_etext = .;

	}	> rom

	/* NOLOAD prevents a large block of zeros to be preserved in flash - would be wasteful */
	.bss	(NOLOAD)	:
	{

		/* Start of .bss - For external use */
		_sbss = .;

		/* Put all the .bss* sections here */
		*(.bss*)

		/* Non-static global scope vars are put into a common section first */
		/* This allows different .c files to ref the same var with a common name */
		/* After its resolved, the symbols end up in RAM */
		/* OMITTING THIS WILL MAKE THE NON-STATIC GLOBAL VARS DISAPPEAR FROM THE OUTPUT BINARY */
		*(COMMON)

		/* End of .bss - For external use */
		_ebss = .;

	} > ram

	.stack (NOLOAD)	:
	{
		/* . (Dot) is a location counter, which represents current offset in memory */
		/* When a section is added, the counter increments. */
		
		/* Aligns the counter to 8 bytes (ensure the stack starts at an 8-byte boundary) */
		. = ALIGN(8);
		
		/* Reserve space for the stack */
		. = . + STACK_SIZE;
		
		/* Do the alignment again */
		. = ALIGN(8);
	} > ram

	/* Flash to SRAM data copy is done here */
	.data	:
	{
		/* Start of .data - For external use */
		_sdata = .;

		/* Put all the .data* sections here */
		*(.data*)

		/* End of .data - For external use */
		_edata = .;

	}	>	ram	AT > rom

	/* For the above, syntax: */
	/* > (DEST_VMA) AT > (ORIGIN_LMA) */
	/* VMA => Virtual memory address */
	/* LMA => Load memory address */

}
