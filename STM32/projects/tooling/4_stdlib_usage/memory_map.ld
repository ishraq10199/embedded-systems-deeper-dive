/* Memory definitions */
MEMORY
{
	/* bootrom - Starts at the beginning of FLASH, size is 16 kB */
	bootrom			(rx)	:	ORIGIN = 0x08000000, LENGTH = 0x00004000

	/* approm - Starts immediately after `bootrom`, size is 240 kB */
	approm			(rx)	:	ORIGIN = 0x08004000, LENGTH = 0x0003C000

	/* shared - Starts at the beginning of SRAM, size is 4 kB*/
	shared			(rwx)	:	ORIGIN = 0x20000000, LENGTH = 0x1000 

	/* ram - Starts immediately after `shared`, size is 16 kB */
	ram					(rwx)	:	ORIGIN = 0x20001000, LENGTH = 0x00004000

	/* execram - Starts immediately after `ram`, size is 16 kB */
	execram			(rwx)	:	ORIGIN = 0x20005000, LENGTH = 0x00004000

}

/* For external use - region sizes and starts */
__bootrom_start__ = ORIGIN(bootrom);
__bootrom_size__  = LENGTH(bootrom);
__shared_start__	= ORIGIN(shared);
__approm_start__  = ORIGIN(approm);
__approm_size__   = LENGTH(approm);
__execram_start__ = ORIGIN(execram);
__execram_size__	= LENGTH(execram);
