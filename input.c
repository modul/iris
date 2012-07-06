#include "conf.h"
#include "input.h"
#include "ad7793.h"

static input_t latest[NUM_AIN] = {0};  
static input_t previous[NUM_AIN] = {0}; 

struct chan {
	uint8_t num;
	uint8_t gain;
};

static struct chan channel[NUM_AIN] = {
	{Fchan, Fgain}, {pchan, pgain}, {schan, sgain}
};

static int mv(int in)
{
	uint64_t result = VREF;
	return (int) ((result*in)>>RESOLUTION);
}

void TC0_IrqHandler()
{
	uint32_t status = TC0->TC_CHANNEL[0].TC_SR;
	static uint8_t n = 0;

	TRACE_DEBUG("n=%u\n", n);

	if ((status = ain_status()) & AD_STAT_NRDY) {
		TRACE_INFO("ADC ch%u not ready (%x)\n", channel[n].num, status);
		return; // try again next time
	}
	else if (status & AD_STAT_ERR) {
		TRACE_ERROR("ADC error ch%u (%x)\n", channel[n].num, status);
	}
	else {
		previous[n] = latest[n];
		latest[n] = ain_read();
		TRACE_DEBUG("ADC read %u (%x)\n", mv(latest[n]), status);

		if (++n == NUM_AIN)
			n = 0;
	}

	ain_start(channel[n].num, channel[n].gain, AD_MODE_SINGLE);
}

void start_sampling()
{
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 1);
}

void stop_sampling()
{
	NVIC_DisableIRQ(TC0_IRQn);
}

input_t get_latest_volt(unsigned index) {
	assert(index < NUM_AIN);
	return mv(latest[index]);
}

input_t get_previous_volt(unsigned index) {
	assert(index < NUM_AIN);
	return mv(previous[index]);
}
