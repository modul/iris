#include <string.h>
#include "board.h"
#include "controller.h"

#define match(a, b) (strcmp(a, b) == 0)

#define LOOPS 2 
#define ADC_EOC_IRQS 3  // = (1 << 0)|(1<<1)|..|(1<<LOOPS-1)

static struct ctrl loops[LOOPS] = {{0}};

static struct {
	ctrlio_t min;
	ctrlio_t max;
	ctrlio_t yinit;
	ctrlio_t uinit;
} conf[LOOPS] = {{0}};

static void init();

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
		switch ((cmd=*line)) {
			case 's':
			case 'g':
				// get ID (0-9 for loop setting)
				if (line[1]-'0' < LOOPS)
					id = line[1]-'0';
				else {
					puts("? id");
					break;
				}
				
				line = line+2;

				// get option (max 7 characters)
				if (sscanf(line, "%s", opt) != 1) {
					puts("? opt");
					break;
				}

				// get arguments (3)
				argc = sscanf(line, "%i%i%i", argv, argv+1, argv+2);

				// check option
				if (match("mode", opt)) {
					if (cmd == 's') {
						if (argc < 1) {
							puts("? arg");
							break;
						}
						if (*argv > STOP || *argv < OFF)
							*argv = OFF;				
						mode(*argv, &loops[id]);
					}
					printf("%u\n", loops[id].mode);
				}
				else if (match("sp", opt)) {
					if (cmd == 's') {
						if (argc < 1) {
							puts("? arg");
							break;
						}
						loops[id].SP = LIMIT(*argv, conf[id].min, conf[id].max);
					}
					printf("%u\n", loops[id].SP);
				}
				else if (match("pv", opt)) {
					if (cmd == 's')
						puts("? read-only");
					printf("%u\n", loops[id]._x);
				}
				else if (match("error", opt)) {
					if (cmd == 's')
						puts("? read-only");
					printf("%u\n", loops[id]._e);
				}
				else if (match("k", opt)) {
					if (cmd == 's') {
						if (argc < 3) {
							puts("? arg");
							break;
						}
						loops[id].Kp = argv[0];
						loops[id].Ki = argv[1];
						loops[id].Kd = argv[2];
					}
					printf("%u %u %u\n", loops[id].Kp, loops[id].Ki, loops[id].Kd);
				}
				if (match("limit", opt)) {
					if (cmd == 's') {
						if (argc < 2) {
							puts("? arg");
							break;
						}
						conf[id].min = argv[0] < argv[1] ? argv[0] : argv[1];
						conf[id].max = argv[0] < argv[1] ? argv[1] : argv[0];
					}
					printf("%u %u\n", conf[id].min, conf[id].max);
				}
				else 
					puts("? opt");
				break;
			case 'l':
			case 'q':
				break;
			default:
				puts("? cmd");
		}
	}
	return 0;
}

void TC1_IrqHandler()
{
	uint8_t i;
	struct ctrl *loop = loops;

	for (i=0; i < LOOPS; i++, loop++) {
		if ((ADC->ADC_CHSR & (1<<i)) == 0 && loop->mode > OFF)
			ADC->ADC_CHER = (1<<i); // enable channel
	}
	ADC_StartConversion(ADC);
}

void ADC_IrqHandler(void)
{
	uint8_t i;
	uint8_t msk;
    uint32_t status;
	struct ctrl *loop = loops;
	ctrlio_t x;

    status = ADC_GetStatus(ADC);

	for (i=0, msk=1; i < LOOPS; i++, msk<<=1, loop++) {
		if (!((status & msk) == msk)) // check End of Conversion flag for channel i
			continue;
			
		x = (ctrlio_t) *(ADC->ADC_CDR+i); // read channel data
		x = LIMIT(x, conf[i].min, conf[i].max);

		if (loop->mode > OFF) {
			control(x, loop);    // run controller
			// write output here
		}
		else {
			ADC->ADC_CHDR = msk; // disable channel
			// reset output here
		}
	}
}

void init()
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
/* vim: set ts=4: */
