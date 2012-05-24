#include "conf.h"
#include "state.h"
#include "input.h"

enum states {SAME, IDLE, READY, SET, GO, ERROR, NUMSTATES};
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
static void do_error();
static void do_conf();
static void do_ack();

static struct state actions[NUMSTATES][NUMEVENTS] = {
/* events      {EV_START, EV_ABORT, EV_LOG, EV_CONF, EV_ESTOP, EV_PTRIG, EV_FTRIG};*/
/* IDLE  */  {{do_press, READY}, {do_abort, IDLE}, {do_log,  IDLE}, {do_conf,  IDLE}, {do_error, ERROR}, {   NULL,   IDLE}, {   NULL,  IDLE}},
/* READY */  {{  do_nok, READY}, {do_abort, IDLE}, {do_log, READY}, { do_nok, READY}, {do_error, ERROR}, {do_stop,    SET}, {   NULL,  READY}},
/* SET   */  {{do_press,    GO}, {do_abort, IDLE}, {do_log,   SET}, { do_nok,   SET}, {do_error, ERROR}, {   NULL,    SET}, {   NULL,  SET}},
/* GO    */  {{  do_nok,    GO}, {do_abort, IDLE}, {do_log,    GO}, { do_nok,    GO}, {do_error, ERROR}, {   NULL,     GO}, {do_vent,  IDLE}},
/* ERROR */  {{  do_nok, ERROR}, {  do_ack, IDLE}, {do_log, ERROR}, { do_nok, ERROR}, {do_error, ERROR}, {   NULL,  ERROR}, {   NULL,  ERROR}},
};

static unsigned _state = IDLE;
static uint16_t soffset = 0;

static void do_nok()
{
	puts("nok");
}

static void do_abort()
{
	puts("ok");
	do_vent();
}

static void do_ack()
{
	// check emergency stop here
	LED_on(ALARM);
	do_vent();
}

static void do_error()
{
	LED_on(ALARM);
	do_vent();
}

static void do_conf()
{
	conf_t cnf = {CONF_INIT};
	char c;
	if (scanf("%u %u %u %u", &cnf.fmax, &cnf.pmax, &cnf.smax, &cnf.gainid) == 4) {
		set_config(cnf);
		printf("ok %u %u %u %u\n", cnf.fmax, cnf.pmax, cnf.smax, cnf.gainid);
	}
	else {
		get_config(&cnf);
		printf("%u %u %u %u\n", cnf.fmax, cnf.pmax, cnf.smax, cnf.gainid);
	}

	while ((c = getchar()) != 10 || c != 0);
}

static void do_log()
{
	printf("%u %u %u %u %u\n", _state, get_latest_volt(F), get_latest_volt(p), get_latest_volt(s), soffset);
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
}

unsigned get_state()
{
	return _state;
}
