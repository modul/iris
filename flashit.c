#include "conf.h"

#define LASTPAGE (IFLASH_ADDR + IFLASH_SIZE - IFLASH_PAGE_SIZE)

void flashit() {
	uint32_t i;
	uint32_t error;
    uint32_t pBuffer[IFLASH_PAGE_SIZE / 4];
    volatile uint32_t *pLastPageData;

    pLastPageData = (volatile uint32_t *) LASTPAGE;

    /* Unlock page */
    TRACE_DEBUG("Unlocking last page (0x%x)\n", LASTPAGE);
    error = FLASHD_Unlock(LASTPAGE, LASTPAGE + IFLASH_PAGE_SIZE, 0, 0);
    assert( !error );

    /* Write page with walking bit pattern (0x00000001, 0x00000002, ...) */
    TRACE_DEBUG("Writing last page with walking bit pattern\n");
    for ( i=0 ; i < (IFLASH_PAGE_SIZE / 4); i++ ) {
        pBuffer[i] = 1 << (i % 32);
	}
    error = FLASHD_Write(LASTPAGE, pBuffer, IFLASH_PAGE_SIZE);
    assert(!error );

    /* Check page contents */
    TRACE_DEBUG("Checking page contents ");
    for (i=0; i < (IFLASH_PAGE_SIZE / 4); i++) {
        TRACE_DEBUG_WP(".");
        assert(pLastPageData[i] == (uint32_t)(1 << (i % 32)));
    }
    TRACE_DEBUG_WP(" ok\n") ;

    /* Lock page */
    TRACE_DEBUG( "Locking last page\n");
    error = FLASHD_Lock(LASTPAGE, LASTPAGE + IFLASH_PAGE_SIZE, 0, 0);
    assert( !error ) ;
}
