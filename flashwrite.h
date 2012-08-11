#ifndef _FLASHWRITE_
#define _FLASHWRITE_

#include <stdint.h>

#define FLASHPAGE (IFLASH_ADDR + IFLASH_SIZE - IFLASH_PAGE_SIZE)

void flashwrite(uint32_t *data, unsigned bytes);

#endif