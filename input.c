#include <string.h>

#include "conf.h"
#include "input.h"
#include "state.h"
#include "flashwrite.h"

static int next = 0;

struct {
	int latest;
	int previous;
} reading[CHANNELS] = {{0}};

void calibrate(int id)
{
	struct chan *channel;
	
	assert(id <= CHANNELS);
	
	if (id == CHANNELS) {
		int i;
		for (i=0; i < CHANNELS; i++) {
			channel = conf_get(i);
			AD7793_calibrate(channel->num, channel->gain);
			TRACE_INFO("ADC ch%u calibrated\n", channel->num);
		}
	}
	else {
		channel = conf_get(id);
		AD7793_calibrate(channel->num, channel->gain);
		TRACE_INFO("ADC ch%u calibrated\n", channel->num);
	}
}

int latest(int id)
{
	assert(id < CHANNELS);
	return reading[id].latest;
}

int previous(int id)
{
	assert(id < CHANNELS);
	return reading[id].previous;
}

void start_sampling()
{
	struct chan *channel = conf_get(next);
	AD7793_start(channel->num, channel->gain);
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
	struct chan *channel = conf_get(next);

	if ((status = AD7793_status()) & AD_STAT_NRDY) {
		TRACE_DEBUG("ADC ch%u not ready\n", channel->num);
	}
	else {
		reading[next].previous = reading[next].latest;
		reading[next].latest = AD7793_read();

		if (status & AD_STAT_ERR) {
			if (reading[next].latest == AD_VMAX || reading[next].latest == AD_VMIN) {
				if (get_error() != EOVL)
					TRACE_WARNING("ADC overload ch%u\n", channel->num);
				send_error(EOVL);
			}
		}
		else if (reading[next].latest >= channel->max) {
			if (get_error() != EMAX)
				TRACE_WARNING("Hit maximum on ch%u (%c)\n", channel->num, CHANNEL_NAME(next));
			send_error(EMAX);
		}
		else if (reading[next].latest <= channel->min) {
			if (get_error() != EMIN)
				TRACE_WARNING("Hit minimum on ch%u (%c)\n", channel->num, CHANNEL_NAME(next));
			send_error(EMIN);
		}
		else if (next == F && reading[next].latest < reading[next].previous/PAR_PEAK)
			send_event(EV_FTRIG);
		else if (next == p && reading[next].latest > PAR_PSET)
			send_event(EV_PTRIG);

		if (++next == CHANNELS)
			next = 0;

		channel = conf_get(next);
		AD7793_start(channel->num, channel->gain);
	}

}

