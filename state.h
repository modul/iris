#ifndef _STATE_
#define _STATE_

#define ERROR_NAME(e) (e == EOK? "ok" : (e == EMIN? "min": (e == EMAX? "max" : (e == EOVL? "ovl" : "?"))))

enum events {EV_CONF, EV_INFO, EV_START, EV_ABORT, EV_LOG, EV_LOAD, EV_STOR, EV_ESTOP, EV_PTRIG, EV_FTRIG, NUMEVENTS};
enum states {IDLE, READY, SET, GO, STOP, NUMSTATES};
enum errors {EOK, EMIN, EMAX, EOVL, NUMERRORS};

void state_send(unsigned ev);
void state_reset();

void state_setError(int id, unsigned err);
unsigned state_getError(int id);
unsigned state_getState();

#endif
