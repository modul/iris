#include "conf.h"
#include "input.h"
#include "state.h"

void setup();

int main() 
{
	char cmd;
	const Pin stop = PIN_STOP;

	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

	setup();
	conf_load();
	input_calibrate(CHANNELS);
	input_start();

	while (1) {
		/* Check emergency stop */
		if (PIO_Get(&stop) == 0) 
			state_send(EV_ESTOP);

		/* Parse command line */
		if (USBC_hasData()) { 
			cmd = getchar();
			if (cmd == 's')
				state_send(EV_START);
			else if (cmd == 'l')
				state_send(EV_LOG);
			else if (cmd == 'a')
				state_send(EV_ABORT);
			else if (cmd == 'i')
				state_send(EV_INFO);
			else if (cmd == 'c')
				state_send(EV_CONF);
			else if (cmd == 'S')
				state_send(EV_STOR);
			else if (cmd == 'L')
				state_send(EV_LOAD);
			
			else if (cmd == 'h') {
				printf("s start\n");
				printf("l log\n");
				printf("a abort/ack\n");
				printf("i info\n");
				printf("c conf\n");
				printf("S stor config\n");
				printf("L load config\n");
				printf("h this help message\n");
			}
			
		}

		/* Display state & error */
		if (timetick() % 1000 == 0) {
			if (!LED_blinking(STATUS))
				LED_blink(STATUS, state_getState());
			if (!LED_blinking(ALARM))
				LED_blink(ALARM, state_getError(CHANNELS));
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
	state_reset();

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
	SPI_ConfigureNPCS(SPI, ADC_CS, ADC_SPICONF);
	SPI_ConfigureNPCS(SPI, MEMORY_CS, MEMORY_SPICONF);
	SPI_Enable(SPI);

	/* LEDs */
	LEDs_configure();
	LED_blink(STATUS, FOREVER);

	/* Configure USB */ 
	USBC_configure();
	TRACE_DEBUG("waiting until USB is fully configured\n");
	while (!USBC_isConfigured());

	setbuf(stdout, NULL);
	LED_blinkstop(STATUS);
	TRACE_INFO("setup done\n");
	LED_on(STATUS);
}
