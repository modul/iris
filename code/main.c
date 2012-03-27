#include <string.h>
#include "board.h"

int main() 
{
	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

	// WDT off
    WDT->WDT_MR = WDT_MR_WDDIS;
	// Tick Config
	TimeTick_Configure(BOARD_MCK);
	// LED PIO Config
	LEDs_configure();
	LED_clr(statusled);

	LED_blink(statusled, 5);
	LED_blink(alarmled, 5);

	while(LED_blinking(statusled));

	USBC_Configure();
	TRACE_DEBUG("waiting until USB is fully configured\n");
	while (!USBC_isConfigured());

	setbuf(stdout, NULL);

	while (1) {
	}
	return 0;
}

/* vim: set ts=4: */
