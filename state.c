#include "conf.h"
#include "state.h"
#include "input.h"

#define OK()  puts("ok")
#define NOK() puts("nok")

#define limit(x, min, max) (x < min? min : (x > max? max : x))

unsigned state = IDLE;
unsigned error[CHANNELS] = {EOK};

static void do_nok();
static void do_log();
static void do_abort();
static void do_press();
static void do_vent();
static void do_stop();
static void do_info();
static void do_conf();
static void do_load();
static void do_stor();

typedef void (*taction_t)();

struct transition {
	taction_t action;
	unsigned next_state;
};

static struct transition table[NUMSTATES][NUMEVENTS] = {
/* event/state EV_CONF,          EV_INFO,         EV_START,          EV_ABORT,         EV_LOG,          EV_LOAD,           EV_ESTOR          EV_ESTOP,        EV_PTRIG,          EV_FTRIG       */
/* IDLE  */  {{do_conf,  IDLE}, {do_info, IDLE},  {do_press, READY}, {do_abort, IDLE}, {do_log,  IDLE}, {do_load,   IDLE}, {do_stor,  IDLE}, {do_vent, STOP}, {   NULL,   IDLE}, {   NULL,  IDLE}},
/* READY */  {{ do_nok, READY}, {do_nok, READY},  {  do_nok, READY}, {do_abort, IDLE}, {do_log, READY}, { do_nok,  READY}, { do_nok, READY}, {do_vent, STOP}, {do_stop,    SET}, {   NULL,  READY}},
/* SET   */  {{ do_nok,   SET}, {do_nok,   SET},  {do_press,    GO}, {do_abort, IDLE}, {do_log,   SET}, { do_nok,    SET}, { do_nok,   SET}, {do_vent, STOP}, {   NULL,    SET}, {   NULL,  SET}},
/* GO    */  {{ do_nok,    GO}, {do_nok,    GO},  {  do_nok,    GO}, {do_abort, IDLE}, {do_log,    GO}, { do_nok,     GO}, { do_nok,    GO}, {do_vent, STOP}, {   NULL,     GO}, {do_vent,  IDLE}},
/* STOP */  {{do_conf,   STOP}, {do_info, STOP},  {  do_nok,  STOP}, {do_abort, IDLE}, {do_log,  STOP}, {do_load,   STOP}, {do_stor,  STOP}, {do_vent, STOP}, {   NULL,   STOP}, {   NULL,  STOP}},
/*              action    next                                                                                            */
};

void state_send(unsigned event)
{
	taction_t action = NULL;

	assert (event < NUMEVENTS);
	if((action = table[state][event].action))
		action();
	state = table[state][event].next_state;
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

static void do_nok()
{
	NOK();
}

static void do_abort()
{
	do_vent();
	if (state == STOP) { // acknowledge error
		const Pin stop = PIN_STOP;
		if (PIO_Get(&stop)) {
			error[F] = EOK;
			error[p] = EOK;
			error[s] = EOK;
			OK();
		}
		else NOK();
	}
	else OK();
}

static void do_log()
{
	printf("%u %u %u %u %i %i %i\n", 
			state, error[F], error[p], error[s], 
			input_latest(F), input_latest(p), input_latest(s));
}

static void do_load()
{
	input_stop();
	conf_load();
	OK();
	input_calibrate(CHANNELS);
	input_start();
}

static void do_stor()
{
	input_stop();
	conf_store();
	input_start();
	OK();
}

static void do_info()
{
	char c;
	int i, tmp;
	struct chan *channel;
	
	input_stop();

	if ((c = getchar()) == 'V') {
		tmp = AD7793_voltmon();
		printf("%u.%uV\n", tmp/1000000, tmp%1000000);
	}
	else if (c == 'T') {
		tmp = AD7793_temperature();
		printf("%u.%uC\n", tmp/10000, tmp%10000);
	}
	else {
		if ((i = CHANNEL_ID(c)) < CHANNELS)
			tmp = i+1;
		else {
			i = 0;
			tmp = CHANNELS;
		}
		
		while (i < tmp) {
			channel = conf_get(i);
			printf("%c %s ch%u %ux %i ... %i %iuV\n", 
				CHANNEL_NAME(i), ERROR_NAME(error[i]), 
				channel->num, 1<<channel->gain,
				channel->min, channel->max,
				input_latest(i));
			i++;
		}
	}

	input_start();
}

static void do_conf()
{
	char c = 0;
	char line[64];
	int args, id, num, gain, min, max;
	struct chan *channel;

	gets(line);
	args = sscanf(line, "%c %u %u %i %i", &c, &num, &gain, &min, &max);
	if (args > 0) {
		id = CHANNEL_ID(c);
		if (id >= CHANNELS)
			NOK();
		else if (args == 1) {
			channel = conf_get(id);
			printf("%c %u %u %i %i\n", c, channel->num, channel->gain, channel->min, channel->max);
		}
		else {
			int tmp;
			input_stop();
			channel = conf_get(id);
			tmp = channel->gain;

			channel->num = limit(num, 0, AD_CHANNELS);

			if (args >= 3) {
				channel->gain = limit(gain, AD_GAIN_MIN, AD_GAIN_MAX);
				if (tmp != channel->gain)
					input_calibrate(id);
			}
			if (args == 4) {
				max = limit(min, 0, AD_VMAX);
				min = limit(-min, AD_VMIN, 0);
				channel->min = min;
				channel->max = max;
			}
			else if (args >= 5) {
				channel->min = limit(min, AD_VMIN, AD_VMAX);
				channel->max = limit(max, channel->min, AD_VMAX);
			}

			printf("ok %c %u %u %i %i\n", c, channel->num, channel->gain, channel->min, channel->max);
			input_start();
		}
	}
	else NOK();
}

static void do_press() 
{
	const Pin pr = PIN_VAL_press;
	const Pin vn = PIN_VAL_vent;
	PIO_Clear(&vn); PIO_Set(&pr);
	OK();
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
