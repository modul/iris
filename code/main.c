#include <string.h>
#include "board.h"
#include "controller.h"

#define MINv 0
#define MAXv MAX

#define match(a, b) (strcmp(a, b) == 0)
#define in_range(x) (x < MAXv && x > MINV)

#define LOOPS 2 
#define LOOPSINIT {CTRL_INIT, CTRL_INIT}

struct ctrl loops[LOOPS] = LOOPSINIT;

void init();

int main() 
{
	char *line;
	char opt[8];
	char cmd;

	int arg[3];
	unsigned id = 0;
	unsigned off = 0;

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
				// get ID (space for main setting, 0-9 for loop setting)
				if (line[1] == ' ')
					id = 0;
				else if (line[1]-'0' < LOOPS)
					id = line[1]-'0' + 1;
				else {
					puts("? id");
					break;
				}
				
				line = line+2;

				// get option (max 7 characters)
				if (sscanf(line, "%s%n", opt, &off) == 0) {
					puts("? opt");
					break;
				}

				line = line+off;

				// check option
				if (match("mode", opt)) {
					if (cmd == 's') {
						if (sscanf(line, "%u", arg) == 0) {
							puts("? arg");
							break;
						}
						if (*arg > STOP || *arg < OFF)
							*arg = OFF;				
						loops[id-1].mode = *arg;
					}
					printf("%u\n", loops[id-1].mode);
				}
				else if (match("sp", opt)) {
					if (cmd == 's') {
						if (sscanf(line, "%u", arg) == 0) {
							puts("? arg");
							break;
						}
						if (*arg < MINv)
							*arg = MINv;
						else if (*arg > MAXv)
							*arg = MAXv;
						loops[id-1].SP = *arg;
					}
					printf("%u\n", loops[id-1].SP);
				}
				else if (match("pv", opt)) {
					if (cmd == 's')
						puts("? read-only");
					printf("%u\n", loops[id-1]._x);
				}
				else if (match("error", opt)) {
					if (cmd == 's')
						puts("? read-only");
					printf("%u\n", loops[id-1]._e);
				}
				else if (match("k", opt)) {
					if (cmd == 's') {
						if (sscanf(line, "%u%u%u", arg, arg+1, arg+2) != 3) {
							puts("? arg");
							break;
						}
						loops[id-1].Kp = arg[0];
						loops[id-1].Ki = arg[1];
						loops[id-1].Kd = arg[2];
					}
					printf("%u %u %u\n", loops[id-1].Kp, loops[id-1].Ki, loops[id-1].Kd);
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
	/*	if (sscanf(line, "%s%n", &cmd, &n) == 1) {
			if (strcmp(cmd, "set") == 0) {
				if (sscanf(line+n, "%u%n", &id, &n) == 1 && n <= LOOPS) {

				}
				else 
					puts("? num");
			}
			else if (strcmp(cmd, "get") == 0) {
				if (sscanf(line+n, "%u%n", &id, &n) == 1 && n <= LOOPS) {

				}
				else 
					puts("? num");
			}
			else if (strcmp(cmd, "stop") == 0) {
			}
			else if (strcmp(cmd, "start") == 0) {
			}
			else if (strcmp(cmd, "ramp") == 0) {
			}
			else if (*cmd == 'c' || *cmd == 'q') {
			}
			else
				puts("? cmd");

		}
		else 
			puts("?");*/
	}
	return 0;
}

void TC1_IrqHandler()
{
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

	/* LED PIO Config */
	LEDs_configure();
	LED_clr(statusled);

	/* USB Console Config */
	USBC_Configure();

}
/* vim: set ts=4: */
