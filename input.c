#include "conf.h"
#include "input.h"
#include "state.h"

#define limit(x, min, max) (x < min? min : (x > max? max : x))

static int next = 0;

struct chan {
	uint8_t num;
	uint8_t gain;
	int max;
	int latest;
	int previous;
};

static struct chan channel[CHANNELS] = {{0}};

void setup_channel(int id, int num, int gain, int max)
{
	assert(id < CHANNELS);
	channel[id].num = limit(num, 0, AD_CHANNELS);
	channel[id].gain = limit(gain, AD_GAIN_MIN, AD_GAIN_MAX);
	channel[id].max = limit(max, 0, AD_VREF-1);

	if (AD7793_calibrate(channel[id].num, channel[id].gain)) {
		TRACE_INFO("ADC ch%u calibration successful\n", channel[id].num);
	}
	else
		TRACE_ERROR("ADC calibration failed on ch%u\n", channel[id].num);
}

void get_channel(int id, int *num, int *gain, int *max)
{
	if (num) *num = channel[id].num;
	if (gain) *gain = channel[id].gain;
	if (max) *max = channel[id].max;
}

int latest(int id)
{
	assert(id < CHANNELS);
	return channel[id].latest;
}

int previous(int id)
{
	assert(id < CHANNELS);
	return channel[id].previous;
}

void start_sampling()
{
	AD7793_start(channel[next].num, channel[next].gain);
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 1);
}

void stop_sampling()
{
	NVIC_DisableIRQ(TC0_IRQn);
}

void TC0_IrqHandler()
{
	uint32_t status = TC0->TC_CHANNEL[0].TC_SR;

	if ((status = AD7793_status()) & AD_STAT_NRDY) {
		TRACE_DEBUG("ADC ch%u not ready\n", channel[next].num);
	}
	else {
		channel[next].previous = channel[next].latest;
		channel[next].latest = AD7793_read();

		if (status & AD_STAT_ERR) {
			if (channel[next].latest > 0) {
				if (get_error() != EOVL)
					TRACE_WARNING("ADC overload ch%u\n", channel[next].num);
				send_error(EOVL);
			}
		}
		else if (channel[next].latest >= channel[next].max) {
			if (get_error() != EMAX)
				TRACE_WARNING("Maximum reached on ch%u (%c)\n", channel[next].num, CHANNEL_NAME(next));
			send_error(EMAX);
		}
		else if (next == F && channel[next].latest < channel[next].previous/PAR_PEAK)
			send_event(EV_FTRIG);
		else if (next == p && channel[next].latest > PAR_PSET)
			send_event(EV_PTRIG);

		if (++next == CHANNELS)
			next = 0;
		AD7793_start(channel[next].num, channel[next].gain);
	}

}

