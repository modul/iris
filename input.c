#include <string.h>
#include "conf.h"
#include "input.h"

#define mV(b) ((b*VREF)>>RESOLUTION)

static input_t next[NUM_AIN] = {0};
static input_t latest[NUM_AIN] = {0};  
static input_t previous[NUM_AIN] = {0}; 

void TC0_IrqHandler()
{
	uint32_t status = TC0->TC_CHANNEL[0].TC_SR;
	status = status;

	ADC_StartConversion(ADC);
}

void ADC_IrqHandler()
{
    uint32_t status;

    status = ADC_GetStatus(ADC);

	if ((status & ADC_ISR_RXBUFF) == ADC_ISR_RXBUFF) {
		memcpy(previous, latest, sizeof(previous));
		memcpy(latest, next, sizeof(latest));
		ADC_ReadBuffer(ADC, (int16_t*) next, NUM_AIN);

		TRACE_DEBUG("Got samples. 0: %u, 1: %u, 2: %u\n", latest[0], latest[1], latest[2]);
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

input_t get_latest_volt(unsigned index) {
	assert(index < NUM_AIN);
	return mV(latest[index]);
}

input_t get_previous_volt(unsigned index) {
	assert(index < NUM_AIN);
	return mV(previous[index]);
}
