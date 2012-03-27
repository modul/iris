#include <string.h>
#include "board.h"
#include "controller.h"

#define LOOPS 2 
#define LOOPSINIT {CTRL_INIT, CTRL_INIT}

struct ctrl loops[LOOPS] = LOOPSINIT;

void init();

int main() 
{
	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

	/* WDT off */
    WDT->WDT_MR = WDT_MR_WDDIS;

	/* configure hardware */
	init();

	TRACE_DEBUG("waiting until USB is fully configured\n");
	while (!USBC_isConfigured());

	LED_blink(statusled, 5);

	setbuf(stdout, NULL);

	while (1) {
	}
	return 0;
}

void TC1_IrqHandler()
{
}

void init()
{
    uint32_t div;
    uint32_t tcclks;

    /* Enable peripheral clock. */
    PMC->PMC_PCER0 = 1 << ID_TC1;

    /* Configure TC */
    TC_FindMckDivisor(SAMPLING_FREQ, BOARD_MCK, &div, &tcclks, BOARD_MCK);
    TC_Configure(TC1, BLINK_TC, tcclks | TC_CMR_CPCTRG);
    TC1->TC_CHANNEL[0].TC_RC = (BOARD_MCK/div) / SAMPLING_FREQ;

    /* Configure and enable interrupt on RC compare */
    NVIC_EnableIRQ((IRQn_Type) ID_TC1);
    TC1->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;

	/* Tick Config */
	TimeTick_Configure(BOARD_MCK);

	/* LED PIO Config */
	LEDs_configure();
	LED_clr(statusled);

	/* USB Console Config */
	USBC_Configure();

}
/* vim: set ts=4: */
