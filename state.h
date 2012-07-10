#ifndef _STATE_
#define _STATE_

enum events {EV_CONF, EV_INFO, EV_START, EV_ABORT, EV_LOG, EV_ESTOP, EV_PTRIG, EV_FTRIG, NUMEVENTS};
enum states {IDLE, READY, SET, GO, ERROR, NUMSTATES};
enum errors {EOK, EFMAX, EPMAX, ESMAX, ESTOP, NUMERRORS};

void send_event(unsigned ev);

void reset_state(); 
unsigned get_state();

void set_error(unsigned flag);
unsigned get_error();

#endif
