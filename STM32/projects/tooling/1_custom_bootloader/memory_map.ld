/* Memory definitions */
MEMORY
{
	/* bootrom - Starts at the beginning of FLASH, size is 16 kB */
	bootrom			(rx)	:	ORIGIN = 0x00000000, LENGTH = 0x00004000

	/* approm - Starts immediately after `bootrom`, size is 240 kB */
	approm			(rx)	:	ORIGIN = 0x00004000, LENGTH = 0x0003C000 

	/* ram - Starts at the beginning of SRAM`, size is 32 kB*/
	ram					(rwx)	:	ORIGIN = 0x20000000, LENGTH = 0x00008000
}

/* For external use */
__bootrom_start__ = ORIGIN(bootrom)
__bootrom_size__  = LENGTH(bootrom)

__approm_start__  = ORIGIN(approm) 
__approm_size__   = LENGTH(approm)    

