#include <string.h>
#include "board.h"
#include "controller.h"

#define match(a, b) (strcmp(a, b) == 0)

#define MINv 0
#define MAXv MAX

#define ADC0 0
#define ADC1 1
#define ADC0m (1 << ADC0)
#define ADC1m (1 << ADC1)
#define ADC_EOC_IRQS ADC0m|ADC1m

static ctrlio_t log = 0;
static struct ctrl loop = CTRL_INIT;

static void init()
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

    /* Initialize ADC */
    ADC_Initialize( ADC, ID_ADC );

    /* startup = 15, prescal = 4, ADC clock = 6.4 MHz */
    ADC_cfgFrequency( ADC, 15, 4 );

    /* Enable ADC interrupt */
    NVIC_EnableIRQ(ADC_IRQn);
    ADC->ADC_IER = ADC_EOC_IRQS;

	/* LED PIO Config */
	LEDs_configure();
	LED_clr(statusled);

	/* USB Console Config */
	USBC_Configure();

}

int main() 
{
	char *line;
	char opt[8];
	char cmd;

	int argc;
	int argv[3];
	unsigned id = 0;

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
		if(!USBC_hasData())
			continue;

		gets(line);
#ifdef ECHO
		puts(line);
#endif
	}
	return 0;
}

void TC1_IrqHandler()
{
	ADC_StartConversion(ADC);
}

void ADC_IrqHandler(void)
{
    uint32_t status;
	ctrlio_t x;

    status = ADC_GetStatus(ADC);

	if ((status & ADC0m) == ADC0m) { // check End of Conversion flag for channel 0
		x = (ctrlio_t) *(ADC->ADC_CDR+ADC0); // read channel data
		x = LIMIT(x, MINv, MAXv);

		if (loop.mode > OFF) {
			control(x, &loop);    // run controller
			// write output here
		}
		else {
			ADC->ADC_CHDR = ADC0m; // disable channel /* do this with state(OFF) or sth*/
			// reset output here
		}
	}

	if ((status & ADC1m) == ADC1m) {
		log = *(ADC->ADC_CDR+ADC1);
		log = LIMIT(log, MINv, MAXv);
	}
}

/* vim: set ts=4: */
