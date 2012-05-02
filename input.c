#include <string.h>
#include "conf.h"

#define TIMER_FREQ (NUM_AIN * SAMPLING_FREQ)

#define mV(b) ((b*VREF)>>RESOLUTION)

static uint16_t next[NUM_AIN] = {0};

uint16_t current[NUM_AIN] = {0};   // current ADC input in mV
uint16_t previous[NUM_AIN] = {0};  // previous ADC input in mV

void TC0_IrqHandler()
{
	uint32_t status = TC0->TC_CHANNEL[0].TC_SR;
	status = status;

	ADC_StartConversion(ADC);
}

void ADC_IrqHandler()
{
	uint8_t i;
    uint32_t status;
	static uint32_t timestamp = 0;

    status = ADC_GetStatus(ADC);

	if ((status & ADC_ISR_RXBUFF) == ADC_ISR_RXBUFF) {
		memcpy(previous, current, NUM_AIN*2);
		for (i=0; i<NUM_AIN; i++)
			current[i] = mV(next[i]);

		TRACE_DEBUG("[%u] Got samples. 0: %umV, 1: %umV, 2: %umV\n", GetTickCount()-timestamp, current[0], current[1], current[2]);
		timestamp = GetTickCount();

		ADC_ReadBuffer(ADC, (int16_t*) next, NUM_AIN);
	}
}

void input_init()
{
    uint32_t div;
    uint32_t tcclks;
	const Pin pins[] = {PINS_ADCIN};

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
}
