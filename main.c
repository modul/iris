#include <string.h>
#include "conf.h"
#include "input.h"
#include "state.h"
#include "ad7793.h"

void setup();

int main() 
{
	char line[64];

	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

	setup();
	start_sampling();

	if (ad_calibrate(Fchan, Fgain))
		TRACE_INFO("Channel F calibrated\n");
	if (ad_calibrate(pchan, pgain))
		TRACE_INFO("Channel p calibrated\n");
	if (ad_calibrate(schan, sgain))
		TRACE_INFO("Channel s calibrated\n");

	/*while(1) {
		int temp = ad_temperature();
		int volt = ad_voltmon();
		TRACE_INFO("--- AVDD: %umV T: %u.%u°C ---\n", volt, temp/10, temp%10);
		
		start_sampling();
		Wait(2000);
		stop_sampling();
	};
*/
	while (1) {
		if (get_latest_volt(Fchan) >= Fmax) {
			TRACE_INFO("FMAX reached.\n");
			set_error(EFMAX);
			send_event(EV_ESTOP);
		}
		if (get_latest_volt(pchan) >= pmax) {
			TRACE_INFO("PMAX reached.\n");
			set_error(EPMAX);
			send_event(EV_ESTOP);
		}
		if (get_latest_volt(schan) >= smax) {
			TRACE_INFO("SMAX reached.\n");
			set_error(ESMAX);
			send_event(EV_ESTOP);
		}
		if (get_latest_volt(pchan) > PAR_PSET) {
			send_event(EV_PTRIG);
		}
		if (get_latest_volt(Fchan) < get_previous_volt(Fchan)/PAR_PEAK) {
			send_event(EV_FTRIG);
		}

		/* Parse command line */
		if (USBC_hasData()) { 
			gets(line);
			if (line[0] == 's') 
				send_event(EV_START);
			else if (line[0] == 'l')
				send_event(EV_LOG);
			else if (line[0] == 'a')
				send_event(EV_ABORT);
			else if (line[0] == 'i') 
				send_event(EV_INFO);
			else if (line[0] == 'c') {
				if (get_state() != IDLE)
					puts("nok");
				else {
					char c = 0;
					int id, num, gain, max;
					if (sscanf(line+1, "%c %u %u %u", &c, &num, &gain, &max) == 4) {
						id = (c == 'F'? F : (c == 'p'? p: (c == 's'? s : c)));
						printf("DBG %c %u %u %u\nDBG %s\n", c, num, gain, max, line);
						if (id >= NUM_AIN)
							puts("nok");
						else {
							setup_channel(id, num, gain, max);
							get_channel(id, &num, &gain, &max);
							printf("ok %c %u %u %u\n", c, num, gain, max);
						}
					}
					else
						puts("nok");
				}
			}
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
	const Pin pins[] = {PINS_VAL, PINS_SPI};

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
