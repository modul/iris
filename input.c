#include "conf.h"
#include "input.h"
#include "ad7793.h"

#define LIMIT(x, min, max) (x < min? min : (x > max? max : x))

static int next = 0;
static int latest[NUM_AIN] = {0};  
static int previous[NUM_AIN] = {0}; 

struct chan {
	uint8_t num;
	uint8_t gain;
	int max;
};

static struct chan channel[NUM_AIN] = {
	{Fchan, Fgain, Fmax}, {pchan, pgain, pmax}, {schan, sgain, smax}
};

void setup_channel(int id, int num, int gain, int max)
{
	channel[id].num = LIMIT(num, 0, NUM_AIN);
	channel[id].gain = LIMIT(gain, AD_GAIN_MIN, AD_GAIN_MAX);
	channel[id].max = LIMIT(max, 0, MAX);
}

void get_channel(int id, int *num, int *gain, int *max)
{
	if (num)
		*num = channel[id].num;
	if (gain)
		*gain = channel[id].gain;
	if (max)
		*max = channel[id].max;
	TRACE_DEBUG("get channel: %u, %u, %u\n", *num, *gain, *max);
}

void start_sampling()
{
	ain_start(channel[next].num, channel[next].gain, AD_MODE_SINGLE);
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 1);
}

void stop_sampling()
{
	NVIC_DisableIRQ(TC0_IRQn);
}

int get_latest_volt(unsigned index) {
	assert(index < NUM_AIN);
	return latest[index];
}

int get_previous_volt(unsigned index) {
	assert(index < NUM_AIN);
	return previous[index];
}

void TC0_IrqHandler()
{
	uint32_t status = TC0->TC_CHANNEL[0].TC_SR;

	TRACE_DEBUG("n=%u\n", next);

	if ((status = ain_status()) & AD_STAT_NRDY) {
		TRACE_INFO("ADC ch%u not ready (%x)\n", channel[next].num, status);
		return; // try again next time
	}
	else if (status & AD_STAT_ERR) {
		TRACE_ERROR("ADC error ch%u (%x)\n", channel[next].num, status);
	}
	else {
		previous[next] = latest[next];
		latest[next] = ain_read();
		TRACE_DEBUG("ADC read %umV (%x)\n", latest[next], status);

		if (++next == NUM_AIN)
			next = 0;
	}

	ain_start(channel[next].num, channel[next].gain, AD_MODE_SINGLE);
}


