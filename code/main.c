#include <string.h>
#include "board.h"

int main() 
{
	int i=0, d=0;
	uint32_t r=250;
	char c, str[32];

	TRACE_INFO("Running at %i MHz\n", SystemCoreClock/1000000);
	if (SystemCoreClock > BOARD_MCK)
		TRACE_WARNING("Slowclock too slow!\n");

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

	printf("%ims> ", r*2);

	while (1) {
		LED_set(statusled); 
		Wait(r-d);
		LED_clr(statusled);
		Wait(r+d);

		if (USBC_hasData()) {
			gets(str);
#ifdef ECHO
			printf("%s\n", str);
#endif

			if (sscanf(str, "%i", &i)) {
				r = i/2;
				d = 0;
				printf("period set\n");
			}
			else if (sscanf(str, "%c%i", &c, &i) == 2 && c == '%') {
				d = i;
				for (; (signed) r - d < 0; d--);
				for (; (signed) r + d < 0; d++);	
				printf("duty off by %ims\n", d);
			}
			else if (sscanf(str, "%c", &c) == 1 && c == 'r') {
				d = 0;
				r = 250;
				printf("reset\n");
			}
			else {
				printf("dafuqq?\n");
				LED_blinkwait(alarmled, 3);
				/* because of previous gets(), flush not neccessary */
				//while ((c=getchar()) != '\n' && c != EOF);
			}

			printf("%ims> ", r*2);
			TRACE_INFO("timing is: %i on, %i off\n", r+d, r-d);
			LED_blink(alarmled, 1);
		}
	}
	return 0;
}

/* vim: set ts=4: */
