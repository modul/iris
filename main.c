#include <string.h>
#include "conf.h"
#include "pga280.h"
#include "input.h"
#include "state.h"

void setup();
void test_spi();

int main() 
{
	char cmd = 0;
	conf_t config;

	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

	setup();
	start_sampling();
	get_config(&config);

	while (1) {
		if (get_latest_volt(F) >= config.fmax) {
			TRACE_INFO("FMAX reached.\n");
			send_event(EV_ESTOP);
			continue;
		}
		if (get_latest_volt(p) >= config.pmax) {
			TRACE_INFO("PMAX reached.\n");
			send_event(EV_ESTOP);
			continue;
		}
		if (get_latest_volt(s) >= config.smax) {
			TRACE_INFO("SMAX reached.\n");
			send_event(EV_ESTOP);
			continue;
		}
		if (get_latest_volt(p) > PAR_PSET) {
			TRACE_INFO("PSET reached.\n");
			send_event(EV_PTRIG);
			continue;
		}
		if (get_latest_volt(F) < get_previous_volt(F)/PAR_PEAK) {
			TRACE_INFO("Fpeak triggered.\n");
			send_event(EV_FTRIG);
			continue;
		}

		/* Parse command line */
		cmd = 0;
		if (USBC_hasData()) { 
			cmd = getchar();
			if (cmd == 's') 
				send_event(EV_START);
			else if (cmd == 'l')
				send_event(EV_LOG);
			else if (cmd == 'a')
				send_event(EV_ABORT);
			else if (cmd == 'c')
				send_event(EV_CONF);
			else
				puts("nok");
		}

		/* Display state */
		if (GetTickCount() % 1000 == 0 && !LED_blinking(STATUS)) {
			LED_blink(STATUS, get_state());
		}
	}
	return 0;
}

void test_spi()
{
	union {
		uint16_t hword; 
		char bytes[2];
	} in, out;

	out.hword = 0x6b6f; // "ok"
	TRACE_DEBUG("Testing SPI, write '%s'\n", out.bytes);
	SPI_Write(SPI, MEMORY_CS|SPI_TDR_LASTXFER, out.hword);
	in.hword = (uint16_t) SPI_Read(SPI);
	TRACE_DEBUG("SPI read: '%s'\n", in.bytes);
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

	/* Configure SPI */
	SPI_Configure(SPI, ID_SPI, SPI_MR_MSTR|SPI_MR_PS|SPI_MR_LLB); // Enable, Reset and set Master mode, variable CS, Loopback (testing)
	SPI_ConfigureNPCS(SPI, PGA_CS, PGA_SPICONF);
	SPI_ConfigureNPCS(SPI, MEMORY_CS, MEMORY_SPICONF);
	SPI_Enable(SPI);

	/* Setup SPI Peripherals */
	PGA_setup();

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
/* vim: set ts=4: */
