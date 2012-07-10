#include <string.h>
#include "conf.h"
#include "input.h"
#include "state.h"
#include "ad7793.h"

void setup();

int main() 
{
	char cmd;
	const Pin stop = PIN_STOP;

	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

	setup();

	setup_channel(F, 0, 0, VREF-2);
	setup_channel(p, 1, 0, VREF-2);
	setup_channel(s, 2, 0, VREF-2);

	while (1) {
		if (PIO_Get(&stop) == 0) {
			set_error(ESTOP);
			send_event(EV_ESTOP);
		}
		if (overload(F)) {
			set_error(EFMAX);
			send_event(EV_ESTOP);
		}
		if (overload(p)) {
			set_error(EPMAX);
			send_event(EV_ESTOP);
		}
		if (overload(s)) {
			set_error(ESMAX);
			send_event(EV_ESTOP);
		}
		if (latest(p) > PAR_PSET) 
			send_event(EV_PTRIG);
		if (latest(F) < previous(F)/PAR_PEAK)
			send_event(EV_FTRIG);

		/* Parse command line */
		if (USBC_hasData()) { 
			cmd = getchar();
			if (cmd == 's')
				send_event(EV_START);
			else if (cmd == 'l')
				send_event(EV_LOG);
			else if (cmd == 'a')
				send_event(EV_ABORT);
			else if (cmd == 'i')
				send_event(EV_INFO);
			else if (cmd == 'c')
				send_event(EV_CONF);
		}

		/* Display state & error */
		if (GetTickCount() % 1000 == 0) {
			if (!LED_blinking(STATUS))
				LED_blink(STATUS, get_state());
			if (!LED_blinking(ALARM))
				LED_blink(ALARM, get_error());
		}
	}
	return 0;
}

void setup()
{
	uint32_t div;
	uint32_t tcclks;
	const Pin pins[] = {PINS_VAL, PIN_STOP, PINS_SPI};

	WDT_Disable(WDT);
	TimeTick_Configure(BOARD_MCK);

	/* PIO Configure */
	PIO_Configure(pins, PIO_LISTSIZE(pins));

	/* Initialize state */
	reset_state();

	/* Enable peripheral clocks */
	PMC_EnablePeripheral(ID_TC0);

	/* Configure TC */
	TC_FindMckDivisor(TIMER_FREQ, BOARD_MCK, &div, &tcclks, BOARD_MCK);
	TC_Configure(TC0, 0, tcclks | TC_CMR_CPCTRG);
	TC0->TC_CHANNEL[0].TC_RC = (BOARD_MCK/div) / TIMER_FREQ;
	TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
	TC_Start(TC0, 0);

	/* Configure SPI */
	SPI_Configure(SPI, ID_SPI, SPI_MR_MSTR|SPI_MR_PS|SPI_MR_MODFDIS);
	SPI_ConfigureNPCS(SPI, AIN_CS, AIN_SPICONF);
	SPI_ConfigureNPCS(SPI, MEMORY_CS, MEMORY_SPICONF);
	SPI_Enable(SPI);

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
	LED_on(STATUS);
}
