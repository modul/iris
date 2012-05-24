#ifndef _STATE_
#define _STATE_

enum events {EV_START, EV_ABORT, EV_LOG, EV_CONF, EV_ESTOP, EV_PTRIG, EV_FTRIG, NUMEVENTS};

void send_event(unsigned ev);
void reset_state(); 
unsigned get_state();

#endif
