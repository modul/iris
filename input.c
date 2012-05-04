#include <string.h>
#include "conf.h"

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

