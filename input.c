#include <string.h>

#include "conf.h"
#include "input.h"
#include "state.h"

static int next = 0;

static struct {
	int latest;
	int previous;
} reading[CHANNELS] = {{0}};

void input_stop()
{
	NVIC_DisableIRQ(TC0_IRQn);
}

void input_start()
{
	struct chan *channel = conf_get(next);
	AD7793_start(channel->num, channel->gain);
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 1);
}

int input_latest(int id)
{
	return reading[id%CHANNELS].latest;
}

int input_previous(int id)
{
	return reading[id%CHANNELS].previous;
}

void input_calibrate(int id)
{
	struct chan *channel;

	if (id == CHANNELS) {
		int i;
		for (i=0; i < CHANNELS; i++) {
			channel = conf_get(i);
			AD7793_calibrate(channel->num, channel->gain);
			TRACE_INFO("ADC ch%u calibrated\n", channel->num);
		}
	}
	else {
		channel = conf_get(id%CHANNELS);
		AD7793_calibrate(channel->num, channel->gain);
		TRACE_INFO("ADC ch%u calibrated\n", channel->num);
	}
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

		if (status & AD_STAT_ERR)
			state_setError(next, EOVL);
		else if (reading[next].latest >= channel->max) 
			state_setError(next, EMAX);
		else if (reading[next].latest <= channel->min) 
			state_setError(next, EMIN);
		else if (next == F && reading[next].latest < reading[next].previous/PAR_PEAK)
			state_send(EV_FTRIG);
		else if (next == p && reading[next].latest > PAR_PSET)
			state_send(EV_PTRIG);

		if (++next == CHANNELS)
			next = 0;

		channel = conf_get(next);
		AD7793_start(channel->num, channel->gain);
	}
}
