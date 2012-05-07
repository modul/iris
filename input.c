#include <string.h>
#include "conf.h"

#define mV(b) ((b*VREF)>>RESOLUTION)

static uint16_t next[NUM_AIN] = {0};

uint16_t latest[NUM_AIN] = {0};   // latest ADC input in mV
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
		memcpy(previous, latest, NUM_AIN*2);
		for (i=0; i<NUM_AIN; i++)
			latest[i] = mV(next[i]);

		TRACE_DEBUG("[%u] Got samples. 0: %umV, 1: %umV, 2: %umV\n", GetTickCount()-timestamp, latest[0], latest[1], latest[2]);
		timestamp = GetTickCount();

		ADC_ReadBuffer(ADC, (int16_t*) next, NUM_AIN);
	}
}

void start_sampling()
{
	NVIC_EnableIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, 0);
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 1);
}

void stop_sampling()
{
	NVIC_DisableIRQ(ADC_IRQn);
	NVIC_DisableIRQ(TC0_IRQn);
}

uint16_t get_latest_volt(unsigned index) {
	assert(index < NUM_AIN);
	return latest[index];
}

uint16_t get_previous_volt(unsigned index) {
	assert(index < NUM_AIN);
	return previous[index];
}
