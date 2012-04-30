#include <string.h>
#include "conf.h"

#define IDLE  0
#define READY 1
#define SET   2
#define GO    3

uint8_t _state = READY;

extern void input_init();

void do_press();
void do_hold();
void do_vent();

void enter(uint8_t new);

int main() 
{
	int t = 0;
	int argc;
	int argv[3];
	char line[32];
	char cmd = 0;
	const Pin pinsout[] = {PINS_VAL};

	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

    WDT->WDT_MR = WDT_MR_WDDIS;
	TimeTick_Configure(BOARD_MCK);
	LEDs_configure();

	/* configure hardware */
	LED_blink(STATUS, FOREVER);
	PIO_Configure(pinsout, PIO_LISTSIZE(pinsout));
	input_init();
	USBC_Configure();
	TRACE_DEBUG("waiting until USB is fully configured\n");
	while (!USBC_isConfigured());
	setbuf(stdout, NULL);
	LED_blinkstop(STATUS);

	enter(IDLE);

	while (1) {
		//TODO always check input limits

		/* Parse command line */
		cmd = 0;
		argc = 0;
		if (USBC_hasData()) {
			gets(line);
			if (*line > 0 && *line != 10) {
				argc = sscanf(line, "%c %u %u %u", &cmd, argv, argv+1, argv+2);
				if (argc > 0) {
					TRACE_DEBUG("got %i valid arguments\n", argc);

					if (cmd == 'a') // abort
						enter(IDLE);
				}
			}
		}
		
		/* Handle state */
		switch (_state) {
			case IDLE:
				if (cmd == 's') // start
					enter(READY);
				//TODO else if ()  // some configuration
				break;

			case READY:
				//TODO if p > pset: enter(SET)
				break;

			case SET:
				if (cmd == 's') // start/continue
					enter(GO);
				break;

			case GO:
				//TODO if F > Ftrig: enter(IDLE)
				break;

			default:
				TRACE_ERROR("invalid state\n");
				enter(IDLE);
		}
		
		/* Display state */
		if ((t=GetTickCount()) % 1000 == 0 && !LED_blinking(STATUS)) 
			LED_blink(STATUS, _state);
	}
	return 0;
}

void do_press() 
{
	const Pin p = PIN_VAL_press;
	const Pin v = PIN_VAL_vent;
	PIO_Clear(&v); PIO_Set(&p);
}

void do_hold()
{
	const Pin pins[] = {PINS_VAL};
	PIO_Clear(pins);
}

void do_vent()
{
	const Pin p = PIN_VAL_press;
	const Pin v = PIN_VAL_vent;
	PIO_Clear(&p); PIO_Set(&v);
}

void enter(uint8_t new) 
{
	LED_blinkstop(STATUS);
	LED_clr(STATUS);
	switch (new) {
		case IDLE:
			do_vent();
			TRACE_DEBUG("entered state IDLE\n");
			break;
		case READY:
			do_press();
			TRACE_DEBUG("entered state READY\n");
			break;
		case SET:
			do_hold();
			TRACE_DEBUG("entered state SET\n");
			break;
		case GO:
			do_press();
			TRACE_DEBUG("entered state GO\n");
			break;
		default:
			TRACE_DEBUG("got invalid state\n");
			enter(IDLE);
			return;
	}
	_state = new;
}

/* vim: set ts=4: */
