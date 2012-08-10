#include "conf.h"
#include "state.h"
#include "input.h"

#define OK()  puts("ok")
#define NOK() puts("nok")

#define limit(x, min, max) (x < min? min : (x > max? max : x))

unsigned state = IDLE;
unsigned error = EOK;

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
	error = EOK;
}

unsigned state_getState()
{
	return state;
}

void state_setError(unsigned err)
{
	error = err;
	state_send(EV_ESTOP);
}

unsigned state_getError()
{
	return error;
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
			error = EOK;
			LED_blinkstop(ALARM);
			LED_off(ALARM);
			OK();
		}
		else NOK();
	}
	else OK();
}

static void do_log()
{
	printf("%u %u %i %i %i\n", 
			state, error, 
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
	int avdd = 0;
	int temp = 0;
	int i;
	struct chan *channel;
	
	input_stop();

	avdd = AD7793_voltmon();
	temp = AD7793_temperature();
	printf("AVdd: %uuV T: %u.%uC\n", avdd, temp/10000, temp%10000);
	for (i=0; i<CHANNELS; i++) {
		channel = conf_get(i);
		printf("%c: ch%u %ux >%i <%i\n", CHANNEL_NAME(i), channel->num, 1<<channel->gain, channel->min, channel->max);
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
