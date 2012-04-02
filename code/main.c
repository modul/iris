#include <string.h>
#include "board.h"
#include "controller.h"

#define STOPPED  (1 << 0)
#define RUN     (1 << 1)
#define HOLD    (1 << 2)
#define RELEASE (1 << 3)

#define MINv 0
#define MAXv MAX

#define AIN0 0
#define AIN1 1
#define NUM_AIN 2

#define PWMOUT_up 0
#define PWMOUT_down 1
#define PWM_FREQ   20
#define PWM_PERIOD 100

ctrlio_t input[NUM_AIN] = {0};
uint8_t _state = STOPPED;

struct ctrl loop = CTRL_INIT;	
struct trip ntrip = {&loop._e, 0, 100, 10};
struct trip rtrip = {&loop._e, 0, 12, 0};

uint32_t releasetime = 1000;
uint32_t dxmax = 1024;

static void init();
static void state(uint8_t new);
static void cli();

int main() 
{
	uint32_t timestamp = 0;

	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

	/* WDT off */
    WDT->WDT_MR = WDT_MR_WDDIS;

	/* Tick Config */
	TimeTick_Configure(BOARD_MCK);

	/* LED PIO Config */
	LEDs_configure();

	/* configure hardware */
	LED_blink(STATUS, FOREVER);
	init();
	LED_blinkstop(STATUS);

	loop.Kp = SCALE(3);
	loop.Ki = SCALE(2);
	loop.Kd = SCALE(0);
	loop.rSlope = 16;
	loop.rSP = MAXv;
	loop.tristate = &rtrip;
	state(STOPPED);

	while (1) {
		cli();
		switch (_state) {
			case STOPPED:
				break;

			case HOLD:
				if (GetTickCount() % 1000)
					LED_blink(STATUS, 1);
				break;

			case RELEASE:
				if (GetTickCount() % 1000)
					LED_blink(STATUS, 2);

				if (timestamp == 0)
					timestamp = GetTickCount() + releasetime;
				else if (GetTickCount() >= timestamp) {
					TRACE_DEBUG("releasetime elapsed\n");
					timestamp = 0;
					state(STOPPED);
				}
				break;

			case RUN:
				if (GetTickCount() % 1000)
					LED_blink(STATUS, 3);
				if (   loop._dx > dxmax \
				    || loop._dx < -dxmax \
					|| loop.mode == NORMAL) // ramp finished
					state(RELEASE);
				break;

			default:
				state(STOPPED);
		}
	}
	return 0;
}

void TC1_IrqHandler()
{
	ADC_StartConversion(ADC);
	ADC_ReadBuffer(ADC, (int16_t*) input, NUM_AIN);
}

void ADC_IrqHandler(void)
{
	//TODO enable TAG option and check
    uint32_t status;
	uint32_t duty = 0;

    status = ADC_GetStatus(ADC);

	if ((status & ADC_ISR_RXBUFF) == ADC_ISR_RXBUFF) {
		if (_state & (RUN|HOLD)) {
			control(LIMIT(input[AIN0], MINv, MAXv), &loop);
			duty = loop.tristate->output * ((loop.output * PWM_PERIOD) / MAX);
			if (loop.tristate->output == 1) {
				PWMC_SetDutyCycle(PWM, PWMOUT_up, 0);
				PWMC_SetDutyCycle(PWM, PWMOUT_down, duty);
			}
			else if (loop.tristate->output == -1) {
				PWMC_SetDutyCycle(PWM, PWMOUT_up, duty);
				PWMC_SetDutyCycle(PWM, PWMOUT_down, 0);
			}
			else {
				PWMC_SetDutyCycle(PWM, PWMOUT_up, 0);
				PWMC_SetDutyCycle(PWM, PWMOUT_down, 0);
			}
		}
	}
}

static void init()
{
    uint32_t div;
    uint32_t tcclks;

	//TODO PIO Configure 

    /* Enable peripheral clocks */
    PMC_EnablePeripheral(ID_TC1);
    PMC_EnablePeripheral(ID_ADC);
    PMC_EnablePeripheral(ID_PWM);

    /* Configure TC */
    TC_FindMckDivisor(SAMPLING_FREQ, BOARD_MCK, &div, &tcclks, BOARD_MCK);
    TC_Configure(TC1, BLINK_TC, tcclks | TC_CMR_CPCTRG);
    TC1->TC_CHANNEL[0].TC_RC = (BOARD_MCK/div) / SAMPLING_FREQ;
	TC_Start(TC1, 0);

    NVIC_EnableIRQ(TC1_IRQn);
    NVIC_SetPriority(TC1_IRQn, 1);
    TC1->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;

    /* Initialize ADC */
    ADC_Initialize(ADC, ID_ADC);
    ADC_cfgFrequency(ADC, 15, 4 ); // startup = 15, prescal = 4, ADC clock = 6.4 MHz

    ADC_EnableChannel(ADC, AIN0);
    ADC_EnableChannel(ADC, AIN1);
	ADC_StartConversion(ADC);
    ADC_ReadBuffer(ADC, (int16_t*) input, NUM_AIN);

    NVIC_EnableIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, 0);
    ADC_EnableIt(ADC,ADC_IER_RXBUFF);

    /* Configure PWMC channels */
    PWMC_ConfigureClocks(PWM_FREQ * PWM_PERIOD, 0, BOARD_MCK);
    PWMC_ConfigureChannelExt(PWM, PWMOUT_up, PWM_CMR_CPRE_CKA, 0, 1, 0, 0, 0, 0);
    PWMC_ConfigureChannelExt(PWM, PWMOUT_down, PWM_CMR_CPRE_CKA, 0, 1, 0, 0, 0, 0);

    PWMC_SetPeriod(PWM, PWMOUT_up, PWM_PERIOD);
    PWMC_SetDutyCycle(PWM, PWMOUT_up, 0);
    PWMC_SetPeriod(PWM, PWMOUT_down, PWM_PERIOD);
    PWMC_SetDutyCycle(PWM, PWMOUT_down, 0);

    PWMC_ConfigureSyncChannel(PWM, (1 << PWMOUT_up)|(1 << PWMOUT_down), PWM_SCM_UPDM_MODE1, 0, 0);
    PWMC_SetSyncChannelUpdatePeriod(PWM, PWM_SCUP_UPR(1));

	/* USB Console Config */
	USBC_Configure();

	TRACE_DEBUG("waiting until USB is fully configured\n");
	while (!USBC_isConfigured());
	setbuf(stdout, NULL);
}

static void state(uint8_t new) 
{
	switch (new) {
		case STOPPED:
			LED_clr(STATUS);
			mode(STOP, &loop);
			PWMC_SetDutyCycle(PWM, PWMOUT_up, 0);
			PWMC_SetDutyCycle(PWM, PWMOUT_down, 0);
			TRACE_DEBUG("set state STOPPED\n");
			break;
		case RUN:
			loop.tristate = &rtrip;
			mode(RAMP, &loop);
			TRACE_DEBUG("set state RUN\n");
			break;
		case HOLD:
			loop.tristate = &ntrip;
			loop.SP = loop._x;
			mode(NORMAL, &loop);
			TRACE_DEBUG("set state HOLD\n");
			break;
		case RELEASE:
			PWMC_SetDutyCycle(PWM, PWMOUT_up, PWM_PERIOD);
			PWMC_SetDutyCycle(PWM, PWMOUT_down, 0);
			mode(STOP, &loop);
			TRACE_DEBUG("set state RELEASE\n");
			break;
		default:
			TRACE_DEBUG("got invalid state\n");
			state(STOPPED);
	}
	_state = new;
}

static void cli()
{
	int argc;
	int argv[3];
	char line[32];
	char cmd = 0;

	if (!USBC_hasData())
		return;

	gets(line);
	argc = sscanf(line, "%c %u %u %u", &cmd, argv, argv+1, argv+2) - 1;
	TRACE_DEBUG("got %i arguments", argc);

	/* commands allowed in all states */
	if (cmd == 's')
		state(STOPPED);
	else if (cmd == 'r')
		state(RELEASE);
	else if (cmd == '?')
		printf("%u %u %u\n", _state, input[AIN0], input[AIN1]);

	/* state specific commands */
	switch (_state) {
		case STOPPED:
			if (cmd == 'g') {
				state(RUN);
			}
			else if (cmd == 'h') {
				state(HOLD);
			}
			else if (cmd == 'e') {
				if (argc == 1) {
					loop.rSP = LIMIT(*argv, MINv, MAXv);
					TRACE_INFO("set ramp endpoint to %u\n", loop.rSP);
				}
				printf("%u\n", loop.rSP);
			}
			else if (cmd == 'v') {
				if (argc == 1) {
					loop.rSlope = *argv;
					TRACE_INFO("set ramp slope to %i\n", loop.rSlope);
				}
				printf("%i\n", loop.rSlope);
			}
			else if (cmd == 'k') {
				if (argc == 3) {
					loop.Kp = argv[0];
					loop.Ki = argv[1];
					loop.Kd = argv[2];
					TRACE_INFO("set PID factors to %u, %u, %u\n", loop.Kp, loop.Ki, loop.Kd);
				}
				printf("%u %u %u\n", loop.Kp, loop.Ki, loop.Kd);
			}
			else if (cmd == 't') {
				if (argc == 1) {
					releasetime = *argv;
					TRACE_INFO("set releasetime to %u\n", releasetime);
				}
				printf("%u\n", releasetime);
			}
			else if (cmd == 'd') {
				if (argc == 2) {
					dxmax = *argv;
					TRACE_INFO("set dxmax to %u\n", dxmax);
				}
				printf("%u\n", dxmax);
			}
			break;

		case HOLD:
			if (cmd == 'w') {
				if (argc == 1) {
					loop.SP = LIMIT(*argv, MINv, MAXv);
					TRACE_INFO("set setpoint to %u\n", loop.SP);
				}
				printf("%u\n", loop.SP);
			}
			break;

		case RUN:
		case RELEASE:
			break;
	}
}
/* vim: set ts=4: */
