#include "conf.h"
#include "state.h"
#include "input.h"

#define OK()  puts("ok")
#define NOK() puts("nok")

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
/* event/state EV_CONF,          EV_INFO,         EV_START,          EV_ABORT,         EV_LOG,          EV_LOAD,           EV_ESTOR          EV_ESTOP,          EV_PTRIG,          EV_FTRIG       */
/* IDLE  */  {{do_conf,  IDLE}, {do_info, IDLE},  {do_press, READY}, {do_abort, IDLE}, {do_log,  IDLE}, {do_load,   IDLE}, {do_stor,  IDLE}, {do_vent, ERROR}, {   NULL,   IDLE}, {   NULL,  IDLE}},
/* READY */  {{ do_nok, READY}, {do_nok, READY},  {  do_nok, READY}, {do_abort, IDLE}, {do_log, READY}, { do_nok,  READY}, { do_nok, READY}, {do_vent, ERROR}, {do_stop,    SET}, {   NULL,  READY}},
/* SET   */  {{ do_nok,   SET}, {do_nok,   SET},  {do_press,    GO}, {do_abort, IDLE}, {do_log,   SET}, { do_nok,    SET}, { do_nok,   SET}, {do_vent, ERROR}, {   NULL,    SET}, {   NULL,  SET}},
/* GO    */  {{ do_nok,    GO}, {do_nok,    GO},  {  do_nok,    GO}, {do_abort, IDLE}, {do_log,    GO}, { do_nok,     GO}, { do_nok,    GO}, {do_vent, ERROR}, {   NULL,     GO}, {do_vent,  IDLE}},
/* ERROR */  {{do_conf, ERROR}, {do_info, ERROR}, {  do_nok, ERROR}, {do_abort, IDLE}, {do_log, ERROR}, {do_load,  ERROR}, {do_stor, ERROR}, {do_vent, ERROR}, {   NULL,  ERROR}, {   NULL,  ERROR}},
/*              action    next                                                                                            */
};

void send_event(unsigned event)
{
	taction_t action = NULL;

	assert (event < NUMEVENTS);
	if((action = table[state][event].action))
		action();
	state = table[state][event].next_state;
}

void reset_state()
{
	do_vent();
	state = IDLE;
	error = EOK;
}

unsigned get_state()
{
	return state;
}

void send_error(unsigned err)
{
	error = err;
	send_event(EV_ESTOP);
}

unsigned get_error()
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
	if (state == ERROR) { // acknowledge error
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
			latest(F), latest(p), latest(s));
}

static void do_load()
{
	stop_sampling();
	load_conf();
	start_sampling();
	OK();
}

static void do_stor()
{
	stop_sampling();
	store_conf();
	start_sampling();
	OK();
}

static void do_info()
{
	int avdd = 0;
	int temp = 0;
	int num, gain, min, max;
	int i;
	
	stop_sampling();

	avdd = AD7793_voltmon();
	temp = AD7793_temperature();
	printf("AVdd: %uuV T: %u.%uC\n", avdd, temp/10000, temp%10000);
	for (i=0; i<CHANNELS; i++) {
		get_channel(i, &num, &gain, &min, &max);
		printf("%c: ch%u %ux >%i <%i\n", CHANNEL_NAME(i), num, 1<<gain, min, max);
	}

	start_sampling();
}

static void do_conf()
{
	char c = 0;
	char line[64];
	int args, id, num, gain, min, max;

	gets(line);
	args = sscanf(line, "%c %u %u %i %i", &c, &num, &gain, &min, &max);
	if (args > 0) {
		id = CHANNEL_ID(c);
		if (id >= CHANNELS)
			NOK();
		else if (args == 1) {
			get_channel(id, &num, &gain, &min, &max);
			printf("%c %u %u %i %i\n", c, num, gain, min, max);
		}
		else if (args == 5) {
			stop_sampling();
			setup_channel(id, num, gain, min, max);
			get_channel(id, &num, &gain, &min, &max);
			printf("ok %c %u %u %i %i\n", c, num, gain, min, max);
			start_sampling();
		}
		else NOK();
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
