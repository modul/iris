#include "conf.h"
#include "output.h"
#include "state.h"

static volatile unsigned state = IDLE;
static volatile unsigned error[CHANNELS] = {EOK};

typedef void (*functionPointer)();

static functionPointer actions[NUMSTATES] = {
	output_vent, output_press, output_stop, output_press, output_vent
};

static unsigned transitions[NUMSTATES][NUMEVENTS] = {
	{STOP, READY, IDLE, IDLE, IDLE},
	{STOP, READY, SET, READY, IDLE},
	{STOP, GO, SET, SET, IDLE},
	{STOP, GO, GO, IDLE, IDLE},
	{STOP, STOP, STOP, STOP, IDLE},
};

void state_send(unsigned event)
{
	functionPointer action = NULL;

	assert (event < NUMEVENTS);
	state = transitions[state][event];
	if ((action = actions[state]))
		action();
}

void state_reset()
{
	output_vent();
	state = IDLE;
	error[F] = EOK;
	error[p] = EOK;
	error[s] = EOK;
}

unsigned state_getState()
{
	return state;
}

void state_setError(int id, unsigned err)
{
	assert(id < CHANNELS);
	error[id] = err;
	state_send(EV_ESTOP);
}

unsigned state_getError(int id)
{
	assert(id < CHANNELS);
	return error[id];
}
