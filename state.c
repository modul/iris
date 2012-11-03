#include "conf.h"
#include "state.h"

static volatile unsigned state = IDLE;
static volatile unsigned error[CHANNELS] = {EOK};

static void do_press();
static void do_stop();
static void do_vent();

typedef void (*functionPointer)();

static functionPointer actions[NUMSTATES] = {
	do_vent, do_press, do_stop, do_press, do_vent
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
	do_vent();
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

unsigned state_isSafe()
{
	if (state == STOP || state == IDLE) 
		return 1;
	return 0;
}

static void do_press() 
{
	const Pin pr = PIN_VAL_press;
	const Pin vn = PIN_VAL_vent;
	PIO_Clear(&vn); PIO_Set(&pr);
}

static void do_stop()
{
	const Pin pins[] = {PINS_VAL};
	PIO_Clear(pins);
}

static void do_vent()
{
	const Pin pr = PIN_VAL_press;
	const Pin vn = PIN_VAL_vent;
	PIO_Clear(&pr); PIO_Set(&vn);
}
