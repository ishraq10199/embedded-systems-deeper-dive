/* Memory definitions */
MEMORY
{
	/* bootrom - Starts at the beginning of FLASH sector 0, size is 16 kB */
	bootrom			(rx)	:	ORIGIN = 0x08000000, LENGTH = 0x00004000

	/* appromA - Starts after bootrom, spans sectors 1 to 4, size is 112 kB */
	appromA			(rx)	:	ORIGIN = 0x08004000, LENGTH = 0x0001C000

	/* appromB - Starts after `appromA`, only in sectors 5, size is 128 kB */
	appromB			(rx)	:	ORIGIN = 0x08020000, LENGTH = 0x00020000
	
	/* metarom - Starts at the beginning of FLASH sector 7, size is 512 B */
	metarom			(rx)	: ORIGIN = 0x08060000, LENGTH = 0x200

	/* shared - Starts at the beginning of SRAM, size is 4 kB*/
	shared			(rwx)	:	ORIGIN = 0x20000000, LENGTH = 0x1000 

	/* ram - Starts immediately after `shared`, size is 32 kB */
	ram					(rwx)	:	ORIGIN = 0x20001000, LENGTH = 0x00008000

}

/* For external use - region sizes and starts */
__bootrom_start__ = ORIGIN(bootrom);
__bootrom_size__  = LENGTH(bootrom);
__shared_start__	= ORIGIN(shared);
__appromA_start__  = ORIGIN(appromA);
__appromA_size__   = LENGTH(appromA);
__appromB_start__  = ORIGIN(appromB);
__appromB_size__   = LENGTH(appromB);
__metarom_start__ = ORIGIN(metarom);
__metarom_size__	= LENGTH(metarom);
