#include "conf.h"
#include "flashit.h"

#define PAGE ((FLASHPAGE - IFLASH_ADDR)/IFLASH_PAGE_SIZE)
#define FLASHKEY 0x5A << 24
#define FLASH_UNLOCK EFC_FCMD_CLB|(PAGE << 8)|FLASHKEY
#define FLASH_WRITE  EFC_FCMD_EWP|(PAGE << 8)|FLASHKEY

static void ram_flashwrite(uint32_t *data, unsigned size) __attribute__((section(".data")));

static void ram_flashwrite(uint32_t *data, unsigned bytes)
{
	uint32_t i = 0;
	uint32_t *destination = (uint32_t *) FLASHPAGE;
	
	for (i=0; i < bytes/4; i++)
		*destination++ = *data++;
	
	EFC->EEFC_FCR = FLASH_UNLOCK;
	while( (EFC->EEFC_FSR & EEFC_FSR_FRDY) == 0);
	EFC->EEFC_FCR = FLASH_WRITE;
	while( (EFC->EEFC_FSR & EEFC_FSR_FRDY) == 0);
}

void flashwrite(uint32_t *data, unsigned bytes)
{
	__disable_irq();
	ram_flashwrite(data, bytes);
	__enable_irq();
}
