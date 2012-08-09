#include <string.h>
#include "conf.h"

#define LASTPAGE (IFLASH_ADDR + IFLASH_SIZE - IFLASH_PAGE_SIZE)
#define LASTPAGENO ((LASTPAGE - IFLASH_ADDR)/IFLASH_PAGE_SIZE)

int channels[3] = {-1, 42, 1024};

void flashread()
{
	volatile uint32_t *buffer = (volatile uint32_t *) LASTPAGE;
	TRACE_DEBUG("%i\n", *buffer);
	buffer++;
	TRACE_DEBUG("%i\n", *buffer);
	buffer++;
	TRACE_DEBUG("%i\n", *buffer);
}

void FlashRam_WriteCommand() __attribute__((section(".data")));

void FlashRam_WriteCommand()
{
	const int KEY = 0x5A << 24;
	uint32_t i = 0;
	uint32_t *page = (uint32_t *) LASTPAGE;
	uint32_t *data = (uint32_t *) channels;
	
	for (i=0; i < IFLASH_PAGE_SIZE/4; i++) {
		*page++ = *data++;
	}
	
	EFC->EEFC_FCR = EFC_FCMD_CLB | (LASTPAGENO << 8) | KEY;
	while( (EFC->EEFC_FSR & EEFC_FSR_FRDY) == 0);
	EFC->EEFC_FCR = EFC_FCMD_EWP | (LASTPAGENO << 8) | KEY;
	while( (EFC->EEFC_FSR & EEFC_FSR_FRDY) == 0);
}

void flashit()
{
	__disable_irq();
	FlashRam_WriteCommand();
	__enable_irq();
}
