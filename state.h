#ifndef _STATE_
#define _STATE_

#define EOK   0
#define EFMAX 1
#define EPMAX 2
#define ESMAX 4
#define ESTOP 8

enum events {EV_START, EV_ABORT, EV_LOG, EV_CONF, EV_ESTOP, EV_PTRIG, EV_FTRIG, NUMEVENTS};
enum states {IDLE, READY, SET, GO, ERROR, NUMSTATES};

void send_event(unsigned ev);

void reset_state(); 
unsigned get_state();

void set_error(unsigned flag);
unsigned get_error();

#endif
