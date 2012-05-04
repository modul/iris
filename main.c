#include <string.h>
#include "conf.h"

#define BUFSIZE 32

#define IDLE  0
#define READY 1
#define SET   2
#define GO    3
#define ERROR 4

#define F AIN0
#define p AIN1
#define s AIN2

#define PAR_PSET 50
#define PAR_PMAX VREF
#define PAR_SMAX VREF
#define PAR_FMAX VREF

uint8_t _state = READY;

extern uint16_t current[NUM_AIN];   // current ADC input in mV
extern uint16_t previous[NUM_AIN];  // previous ADC input in mV

extern void input_init();

void do_press();
void do_hold();
void do_vent();

void enter(uint8_t new);

int main() 
{
	int argc = 0;
	int argv[3] = {0};
	char cmd = 0;
	char line[BUFSIZE];

	const Pin pinsout[] = {PINS_VAL};
	uint16_t soffset = 0;

	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

    WDT->WDT_MR = WDT_MR_WDDIS;
	TimeTick_Configure(BOARD_MCK);
	PIO_Configure(pinsout, PIO_LISTSIZE(pinsout));
	enter(IDLE);
	input_init();

	LEDs_configure();
	LED_blink(STATUS, FOREVER);
	USBC_Configure();
	TRACE_DEBUG("waiting until USB is fully configured\n");
	while (!USBC_isConfigured());
	setbuf(stdout, NULL);
	LED_blinkstop(STATUS);


	while (1) {
		if (current[F] >= PAR_FMAX) {
			TRACE_INFO("FMAX reached.\n");
			enter(ERROR);
		}
		if (current[p] >= PAR_PMAX) {
			TRACE_INFO("PMAX reached.\n");
			enter(ERROR);
		}
		if (current[s] >= PAR_SMAX) {
			TRACE_INFO("SMAX reached.\n");
			enter(ERROR);
		}

		/* Parse command line */
		cmd = 0;
		argc = 0;
		if (USBC_hasData()) {
			if (fgets(line, BUFSIZE, stdin) == NULL) {
				perror("cli error");
			}
			else {
				argc = sscanf(line, "%c %u %u %u", &cmd, argv, argv+1, argv+2);
				if (argc > 0) {
					TRACE_DEBUG("got %i valid arguments\n", argc);

					if (cmd == 'a') { // abort or acknowledge ERROR
						puts("ok");
						enter(IDLE);
					}
					else if (cmd == 'l') // log
						printf("%u %u %u %u %u\n", _state, current[F], current[p], current[s], soffset);
				}
			}
		}

		/* Handle state */
		switch (_state) {
			case IDLE:
				if (cmd == 's') { // start
					soffset = 0;
					enter(READY);
				}
				//TODO some configuration here
				break;

			case READY:
				if (current[p] > PAR_PSET) {
					soffset = current[s];
					enter(SET);
				}
				break;

			case SET:
				if (cmd == 's') // start/continue
					enter(GO);
				break;

			case GO:
				if (current[F] <= previous[F]/2)
					enter(IDLE);
				break;

			default:
				TRACE_ERROR("invalid state\n");
				enter(IDLE);
		}
		
		/* Display state */
		if (GetTickCount() % 1000 == 0 && !LED_blinking(STATUS)) 
			LED_blink(STATUS, _state);
	}
	return 0;
}

void do_press() 
{
	const Pin pr = PIN_VAL_press;
	const Pin vn = PIN_VAL_vent;
	PIO_Clear(&vn); PIO_Set(&pr);
}

void do_hold()
{
	const Pin pins[] = {PINS_VAL};
	PIO_Clear(pins);
}

void do_vent()
{
	const Pin pr = PIN_VAL_press;
	const Pin vn = PIN_VAL_vent;
	PIO_Clear(&pr); PIO_Set(&vn);
}

void enter(uint8_t new) 
{
	LED_blinkstop(STATUS);
	LED_clr(STATUS);
	switch (new) {
		case ERROR:
			do_vent();
			LED_clr(ALARM);
			TRACE_DEBUG("entered state ERROR\n");
			break;
		case IDLE:
			do_vent();
			TRACE_DEBUG("entered state IDLE\n");
			if (_state == ERROR) // ERROR was acknowledged, turn of alarm
				LED_set(ALARM);
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
