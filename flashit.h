#ifndef _FLASHWRITE_
#define _FLASHWRITE_

#define FLASHPAGE (IFLASH_ADDR + IFLASH_SIZE - IFLASH_PAGE_SIZE)

void flashwrite(uint32_t *data, unsigned bytes);

#endif