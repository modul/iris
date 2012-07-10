#include "conf.h"
#include "state.h"
#include "input.h"

static struct _proc {
	unsigned state;
	unsigned error;
} proc = {IDLE, EOK};

static void do_nok();
static void do_log();
static void do_abort();
static void do_press();
static void do_vent();
static void do_stop();
//static void do_conf();


typedef void (*taction_t)();

struct transition {
	taction_t action;
	unsigned next_state;
};

static struct transition table[NUMSTATES][NUMEVENTS] = {
/* event/state EV_START,          EV_ABORT,         EV_LOG,          EV_ESTOP,          EV_PTRIG,          EV_FTRIG       */
/* IDLE  */  {{do_press, READY}, {do_abort, IDLE}, {do_log,  IDLE}, {do_vent, ERROR}, {   NULL,   IDLE}, {   NULL,  IDLE}},
/* READY */  {{  do_nok, READY}, {do_abort, IDLE}, {do_log, READY}, {do_vent, ERROR}, {do_stop,    SET}, {   NULL,  READY}},
/* SET   */  {{do_press,    GO}, {do_abort, IDLE}, {do_log,   SET}, {do_vent, ERROR}, {   NULL,    SET}, {   NULL,  SET}},
/* GO    */  {{  do_nok,    GO}, {do_abort, IDLE}, {do_log,    GO}, {do_vent, ERROR}, {   NULL,     GO}, {do_vent,  IDLE}},
/* ERROR */  {{  do_nok, ERROR}, {do_abort, IDLE}, {do_log, ERROR}, {do_vent, ERROR}, {   NULL,  ERROR}, {   NULL,  ERROR}},
/*              action    next                                                                                            */
};

void send_event(unsigned event)
{
	taction_t action = NULL;

	assert (event < NUMEVENTS);
	if((action = table[proc.state][event].action))
		action();
	proc.state = table[proc.state][event].next_state;
}

void reset_state()
{
	do_vent();
	proc.state = IDLE;
	proc.error = EOK;
}

unsigned get_state()
{
	return proc.state;
}

void set_error(unsigned flag)
{
	proc.error |= flag;
}

unsigned get_error()
{
	return proc.error;
}

static void do_nok()
{
	puts("nok");
}

static void do_abort()
{
	do_vent();
	puts("ok");
	if (proc.state == ERROR) { // acknowledge error
		// check emergency stop here
		proc.error = EOK;
		LED_blinkstop(ALARM);
		LED_off(ALARM);
	}
}

/*static void do_conf()
{
	conf_t cnf = {CONF_INIT};
	char buf[32];

	fgets(buf, 32, stdin);
	if (sscanf(buf, "%u %u %u %u", &(cnf.fmax), &(cnf.pmax), &(cnf.smax), &(cnf.gainid)) == 4) {
		set_config(cnf);
		printf("ok ");
	}
	else {
		get_config(&cnf);
	}

	printf("%u %u %u %u\n", cnf.fmax, cnf.pmax, cnf.smax, cnf.gainid);
}
*/
static void do_log()
{
	printf("%u %u %u %u %u %u\n", proc.state, proc.error, get_latest_volt(Fchan), get_latest_volt(pchan), get_latest_volt(schan));
}

static void do_press() 
{
	const Pin pr = PIN_VAL_press;
	const Pin vn = PIN_VAL_vent;
	PIO_Clear(&vn); PIO_Set(&pr);
	puts("ok");
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
