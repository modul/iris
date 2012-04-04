#include <string.h>
#include "conf.h"

// Timer must fire once for each conversion
#define TIMER_FREQ (NUM_AIN * SAMPLING_FREQ)

#define STOPPED  (1 << 0)
#define RUN     (1 << 1)
#define HOLD    (1 << 2)
#define RELEASE (1 << 3)

//TODO config struct
#define MINv 0
#define MAXv MAX

uint16_t input[NUM_AIN] = {0};
uint8_t _state = STOPPED;

static const Pin vpins[] = {PINS_VAL};

#define PRESS() PIO_Clear(vpins+VAL_vent); PIO_Set(vpins+VAL_press)
#define VENT() PIO_Clear(vpins+VAL_press); PIO_Set(vpins+VAL_vent)

static void init();
static void state(uint8_t new);
static void cli();

int main() 
{
	//uint32_t timestamp = 0;

	TRACE_INFO("Running at %i MHz\n", BOARD_MCK/1000000);

    WDT->WDT_MR = WDT_MR_WDDIS;
	TimeTick_Configure(BOARD_MCK);
	LEDs_configure();

	/* configure hardware */
	LED_blink(STATUS, FOREVER);
	init();
	LED_blinkstop(STATUS);

	state(STOPPED);

	while (1) {
		cli();
	}
	return 0;
}

void TC0_IrqHandler()
{
	uint32_t status = TC0->TC_CHANNEL[0].TC_SR;
	status = status;

	ADC_StartConversion(ADC);
}

void ADC_IrqHandler()
{
    uint32_t status;
	static uint32_t timestamp = 0;

    status = ADC_GetStatus(ADC);
	
	TRACE_DEBUG("[%u] Got samples. 0: %u, 1: %u, 2: %u\n", GetTickCount()-timestamp, input[0], input[1], input[2]);
	timestamp = GetTickCount();

	if ((status & ADC_ISR_RXBUFF) == ADC_ISR_RXBUFF) {
		ADC_ReadBuffer(ADC, (int16_t*) input, NUM_AIN);
	}
}

static void init()
{
    uint32_t div;
    uint32_t tcclks;
	const Pin pins[] = {PINS_VAL, PINS_ADCIN};

	/* PIO Configure */
	PIO_Configure(pins, PIO_LISTSIZE(pins));

    /* Enable peripheral clocks */
    PMC_EnablePeripheral(ID_TC0);
    PMC_EnablePeripheral(ID_ADC);

    /* Configure TC */
    TC_FindMckDivisor(TIMER_FREQ, BOARD_MCK, &div, &tcclks, BOARD_MCK);
    TC_Configure(TC0, 0, tcclks | TC_CMR_CPCTRG);
    TC0->TC_CHANNEL[0].TC_RC = (BOARD_MCK/div) / TIMER_FREQ;
    TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
	TC_Start(TC0, 0);

    /* Initialize ADC */
    ADC_Initialize(ADC, ID_ADC);
    ADC_cfgFrequency(ADC, 4, 1 ); // startup = 64 ADC periods, prescal = 1, ADC clock = 12 MHz
	ADC->ADC_CHER = (1<<AIN0)|(1<<AIN1)|(1<<AIN2);
    ADC->ADC_IER  = ADC_IER_RXBUFF;

	/* Enable Interrupts */
    NVIC_EnableIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, 0);
    NVIC_EnableIRQ(TC0_IRQn);
    NVIC_SetPriority(TC0_IRQn, 1);

	/* USB Console Configuration */
	USBC_Configure();

	TRACE_DEBUG("waiting until USB is fully configured\n");
	while (!USBC_isConfigured());
	setbuf(stdout, NULL);

	TRACE_INFO("configured\n");
}

static void state(uint8_t new) 
{
	LED_blinkstop(STATUS);
	switch (new) {
		case STOPPED:
			LED_clr(STATUS);
			TRACE_DEBUG("set state STOPPED\n");
			break;
		case RUN:
			TRACE_DEBUG("set state RUN\n");
			break;
		case HOLD:
			TRACE_DEBUG("set state HOLD\n");
			break;
		case RELEASE:
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
	if (*line == 0 || *line == 10)
		return;

	argc = sscanf(line, "%c %u %u %u", &cmd, argv, argv+1, argv+2) - 1;
	if (argc > 0)
		TRACE_DEBUG("got %i arguments\n", argc);
}
/* vim: set ts=4: */
