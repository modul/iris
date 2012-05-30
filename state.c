#include "conf.h"
#include "state.h"
#include "input.h"

typedef void (*paction_t)();

struct state {
	paction_t action;
	unsigned next;
};

static void do_nok();
static void do_log();
static void do_abort();
static void do_press();
static void do_vent();
static void do_stop();
static void do_conf();

static unsigned _state = IDLE;
static unsigned _error = EOK;
static input_t soffset = 0;

static struct state actions[NUMSTATES][NUMEVENTS] = {
/* event/state EV_START,          EV_ABORT,         EV_LOG,          EV_CONF,          EV_ESTOP,          EV_PTRIG,          EV_FTRIG       */
/* IDLE  */  {{do_press, READY}, {do_abort, IDLE}, {do_log,  IDLE}, {do_conf,  IDLE}, {do_vent, ERROR}, {   NULL,   IDLE}, {   NULL,  IDLE}},
/* READY */  {{  do_nok, READY}, {do_abort, IDLE}, {do_log, READY}, { do_nok, READY}, {do_vent, ERROR}, {do_stop,    SET}, {   NULL,  READY}},
/* SET   */  {{do_press,    GO}, {do_abort, IDLE}, {do_log,   SET}, { do_nok,   SET}, {do_vent, ERROR}, {   NULL,    SET}, {   NULL,  SET}},
/* GO    */  {{  do_nok,    GO}, {do_abort, IDLE}, {do_log,    GO}, { do_nok,    GO}, {do_vent, ERROR}, {   NULL,     GO}, {do_vent,  IDLE}},
/* ERROR */  {{  do_nok, ERROR}, {do_abort, IDLE}, {do_log, ERROR}, { do_nok, ERROR}, {do_vent, ERROR}, {   NULL,  ERROR}, {   NULL,  ERROR}},
/*              action    next                                                                                                               */
};

void send_event(unsigned ev)
{
	paction_t action = NULL;

	assert (ev < NUMEVENTS);
	action = actions[_state][ev].action;
	if (action)
		action();
	_state = actions[_state][ev].next;
}

void reset_state()
{
	do_vent();
	_state = IDLE;
	_error = EOK;
}

unsigned get_state()
{
	return _state;
}

void set_error(unsigned flag)
{
	_error |= flag;
}

unsigned get_error()
{
	return _error;
}

static void do_nok()
{
	puts("nok");
}

static void do_abort()
{
	do_vent();
	puts("ok");
	if (_state == ERROR) { // acknowledge error
		// check emergency stop here
		_error = EOK;
		LED_blinkstop(ALARM);
		LED_off(ALARM);
	}
}

static void do_conf()
{
	conf_t cnf = {CONF_INIT};
	char c;
	if (scanf("%u %u %u %u", &cnf.fmax, &cnf.pmax, &cnf.smax, &cnf.gainid) == 4) {
		set_config(cnf);
		printf("ok ");
	}
	else {
		get_config(&cnf);
	}

	printf("%u %u %u %u\n", cnf.fmax, cnf.pmax, cnf.smax, cnf.gainid);
}

static void do_log()
{
	printf("%u %u %u %u %u %u\n", _state, _error, get_latest_volt(F), get_latest_volt(p), get_latest_volt(s), soffset);
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
	soffset = get_latest_volt(s);
}

static void do_vent()
{
	const Pin pr = PIN_VAL_press;
	const Pin vn = PIN_VAL_vent;
	PIO_Clear(&pr); PIO_Set(&vn);
}
