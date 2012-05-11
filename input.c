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
    uint32_t status;
#ifdef TRACE_LEVEL_DEBUG
	static uint32_t timestamp = 0;
#endif

    status = ADC_GetStatus(ADC);

	if ((status & ADC_ISR_RXBUFF) == ADC_ISR_RXBUFF) {
		memcpy(previous, latest, NUM_AIN*2);
		memcpy(latest, next, NUM_AIN*2);
		ADC_ReadBuffer(ADC, (int16_t*) next, NUM_AIN);

#ifdef TRACE_LEVEL_DEBUG
		TRACE_DEBUG("[%u] Got samples. 0: %u, 1: %u, 2: %u\n", GetTickCount()-timestamp, latest[0], latest[1], latest[2]);
		timestamp = GetTickCount();
#endif
	}
}

void start_sampling()
{
	NVIC_EnableIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, 0);
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 1);

	ADC_ReadBuffer(ADC, (int16_t*) next, NUM_AIN);
}

void stop_sampling()
{
	NVIC_DisableIRQ(ADC_IRQn);
	NVIC_DisableIRQ(TC0_IRQn);
}

uint16_t get_latest_volt(unsigned index) {
	assert(index < NUM_AIN);
	return mV(latest[index]);
}

uint16_t get_previous_volt(unsigned index) {
	assert(index < NUM_AIN);
	return mV(previous[index]);
}
