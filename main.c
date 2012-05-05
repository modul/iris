#include <string.h>
#include "conf.h"
#include "input.h"

#define BUFSIZE 32

#define IDLE  0
#define READY 1
#define SET   2
#define GO    3
#define ERROR 4


#define in(state) (_state == state)

uint8_t _state = READY;

void setup();
void enter(uint8_t new);

void do_press();
void do_hold();
void do_vent();

int main() 
{
	int argc = 0;
	int argv[3] = {0};
	char cmd = 0;
	char line[BUFSIZE];

	uint16_t soffset = 0;

	conf_t config = {CONF_INIT};

	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

	setup();
	start_sampling();

	while (1) {
		if (current[config.F] >= config.fmax && !in(ERROR)) {
			TRACE_INFO("FMAX reached.\n");
			enter(ERROR);
		}
		if (current[config.P] >= config.pmax && !in(ERROR)) {
			TRACE_INFO("PMAX reached.\n");
			enter(ERROR);
		}
		if (current[config.S] >= config.smax && !in(ERROR)) {
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
						printf("%u %u %u %u %u\n", _state, current[config.F], current[config.P], current[config.S], soffset);
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
				if (current[config.P] > config.pset) {
					soffset = current[config.S];
					enter(SET);
				}
				break;

			case SET:
				if (cmd == 's') // start/continue
					enter(GO);
				break;

			case GO:
				if (current[config.F] <= previous[config.F]/config.fpeakdiv)
					enter(IDLE);
				break;

			case ERROR:
				break;

			default:
				TRACE_ERROR("invalid state\n");
				enter(ERROR);
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
	LED_on(STATUS);
	switch (new) {
		case ERROR:
			do_vent();
			LED_on(ALARM);
			TRACE_INFO("entered state ERROR\n");
			break;
		case IDLE:
			do_vent();
			TRACE_INFO("entered state IDLE\n");
			if (in(ERROR)) // ERROR was acknowledged, turn of alarm
				LED_off(ALARM);
			break;
		case READY:
			do_press();
			TRACE_INFO("entered state READY\n");
			break;
		case SET:
			do_hold();
			TRACE_INFO("entered state SET\n");
			break;
		case GO:
			do_press();
			TRACE_INFO("entered state GO\n");
			break;
		default:
			TRACE_INFO("got invalid state\n");
			enter(IDLE);
			return;
	}
	_state = new;
}

void setup()
{
	uint32_t div;
	uint32_t tcclks;
	const Pin pins[] = {PINS_VAL, PINS_ADCIN};

	WDT_Disable(WDT);
	TimeTick_Configure(BOARD_MCK);

	/* PIO Configure */
	PIO_Configure(pins, PIO_LISTSIZE(pins));

	/* Initialize state */
	enter(IDLE);

	/* Enable peripheral clocks */
	PMC_EnablePeripheral(ID_TC0);
	PMC_EnablePeripheral(ID_ADC);

	/* Configure TC */
	TC_FindMckDivisor(TIMER_FREQ, BOARD_MCK, &div, &tcclks, BOARD_MCK);
	TC_Configure(TC0, 0, tcclks | TC_CMR_CPCTRG);
	TC0->TC_CHANNEL[0].TC_RC = (BOARD_MCK/div) / TIMER_FREQ;
	TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
	TC_Start(TC0, 0);

	/* Configure ADC */
	ADC_Initialize(ADC, ID_ADC);
	ADC_cfgFrequency(ADC, 4, 1 ); // startup = 64 ADC periods, prescal = 1, ADC clock = 12 MHz
	ADC->ADC_CHER = (1<<AIN0)|(1<<AIN1)|(1<<AIN2);
	ADC->ADC_IER  = ADC_IER_RXBUFF;

	/* LEDs */
	LEDs_configure();
	LED_blink(STATUS, FOREVER);

	/* Configure USB */ 
	USBC_Configure();
	TRACE_DEBUG("waiting until USB is fully configured\n");
	while (!USBC_isConfigured());

	setbuf(stdout, NULL);
	LED_blinkstop(STATUS);
	TRACE_INFO("setup done\n");
}
/* vim: set ts=4: */
